// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Components/BoxComponent.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickSaveFramePackage()
{
	//It is not necessary for the client to to save packages
	if(!IsValid(Character) || !Character->HasAuthority()) return;
	
	if(FramesHistory.Num() <= 1)	//The first 2 FramePackages
	{
		FFramePackage ThisFramePackage;
		SaveFramePackage(ThisFramePackage);
		FramesHistory.AddHead(ThisFramePackage);
	}
	else
	{
		float HistoryLength = FramesHistory.GetHead()->GetValue().Time - FramesHistory.GetTail()->GetValue().Time;
		while(HistoryLength > MaxRecordTime)
		{
			FramesHistory.RemoveNode(FramesHistory.GetTail());
			HistoryLength = FramesHistory.GetHead()->GetValue().Time - FramesHistory.GetTail()->GetValue().Time;
		}
		
		FFramePackage ThisFramePackage;
		SaveFramePackage(ThisFramePackage);
		FramesHistory.AddHead(ThisFramePackage);

		//To use for debug only
		//ShowFramePackage(ThisFramePackage, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& InFramePackage)
{
	Character = !IsValid(Character) ? Cast<AEndlessBetrayalCharacter>(GetOwner()) : Character;
	if(IsValid(Character) && !Character->HitCollisionBoxes.IsEmpty())
	{
		//As we're calling this function only on the server, GetWorld()->GetTimeSeconds() gives the official server time
		InFramePackage.Time = GetWorld()->GetTimeSeconds();
		InFramePackage.Character = Character;
		
		FBoxInformation BoxInformation;
		for (const auto& HitCollisionBox : Character->HitCollisionBoxes)
		{
			BoxInformation.Location = HitCollisionBox.Value->GetComponentLocation();
			BoxInformation.Rotation = HitCollisionBox.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitCollisionBox.Value->GetUnscaledBoxExtent();

			InFramePackage.HitBoxInformationMap.Add(HitCollisionBox.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Color)
{
	for (const auto& HitBox : PackageToShow.HitBoxInformationMap)
	{
		DrawDebugBox(GetWorld(), HitBox.Value.Location, HitBox.Value.BoxExtent, FQuat(HitBox.Value.Rotation), Color, false, 3.0f);
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance , 0.0f, 1.0f);

	FFramePackage FrameToReturn;
	FrameToReturn.Time = HitTime;

	for(auto& YoungerPair : YoungerFrame.HitBoxInformationMap)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInformationMap[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInformationMap[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.0f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.0f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent; //Same as if we were using OlderBox (box extent are not changing during the game)

		FrameToReturn.HitBoxInformationMap.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return FrameToReturn;
}

FServerSideRewindResults ULagCompensationComponent::ConfirmHit(const FFramePackage& FrameToCheck, AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(!ensureAlways(IsValid(HitCharacter))) FServerSideRewindResults();
	
	//We're caching the location of all the boxes so we can move them back later
	FFramePackage CurrentFrame;
	
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, FrameToCheck);

	ToggleCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	//Enable collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //So we can line trace to verify a HeadShot
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();

	if(IsValid(World))
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if(ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			ToggleCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

			return FServerSideRewindResults{true, true};
		}
		else //Didn't hit the head, check the rest of the boxes
		{
			for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(IsValid(HitBoxPair.Value))
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

			if(ConfirmHitResult.bBlockingHit) //We've hit the character
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				ToggleCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResults{true, false};
			}
		}
	}

	//No confirmed hit
	ResetHitBoxes(HitCharacter, CurrentFrame);
	ToggleCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResults{false, false};
}

void ULagCompensationComponent::CheckShotgunShots(TMap<AEndlessBetrayalCharacter*, uint32>& MapShots, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	const UWorld* World = GetWorld();
	for (const FVector_NetQuantize& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		if(IsValid(World))
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
			
			AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(ConfirmHitResult.GetActor());
			if(IsValid(HitCharacter)) //It is a valid shot!
			{
				if(MapShots.Contains(HitCharacter))
				{
					++MapShots[HitCharacter];
				}
				else
				{
					MapShots.Emplace(HitCharacter, 1);
				}
			}
			
		}
	}
}

FShotgunServerSideRewindResults ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	FShotgunServerSideRewindResults ShotgunServerSideRewindResults;
	TArray<FFramePackage> CurrentFrames;
	
	for (const FFramePackage& FrameToCheck : FramePackages)
	{
		if(!FrameToCheck.Character) return FShotgunServerSideRewindResults();
		
		FFramePackage CurrentFrame;
		CurrentFrame.Character = FrameToCheck.Character;
		
		CacheBoxPositions(FrameToCheck.Character, CurrentFrame);
		MoveBoxes(FrameToCheck.Character, FrameToCheck);

		ToggleCharacterMeshCollision(FrameToCheck.Character, ECollisionEnabled::NoCollision);

		//Enable collision for the head first
		UBoxComponent* HeadBox = FrameToCheck.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //So we can line trace to verify a HeadShot
		HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		CurrentFrames.Add(CurrentFrame);
	}

	//Next loop Checks for headshots!
	CheckShotgunShots(ShotgunServerSideRewindResults.HeadShotsMap, TraceStart, HitLocations);

	//Enabling all box collisions and disabling head boxes collisions
	for (const FFramePackage& FrameToCheck : FramePackages)
	{
		if(!FrameToCheck.Character) return FShotgunServerSideRewindResults();

		for (auto& HitCollisionBox : FrameToCheck.Character->HitCollisionBoxes)
		{
			if(IsValid(HitCollisionBox.Value))
			{
				//Enabling all boxes collision
				HitCollisionBox.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitCollisionBox.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			}
			if(FrameToCheck.Character)
			{
				UBoxComponent* HeadBox = FrameToCheck.Character->HitCollisionBoxes[FName("head")];
				HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	//Check for body shots
	CheckShotgunShots(ShotgunServerSideRewindResults.BodyShots, TraceStart, HitLocations);

	for (const FFramePackage& CurrentFrame : CurrentFrames)
	{
		ResetHitBoxes(CurrentFrame.Character, CurrentFrame);
		ToggleCharacterMeshCollision(CurrentFrame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunServerSideRewindResults;
}

void ULagCompensationComponent::ToggleCharacterMeshCollision(AEndlessBetrayalCharacter* HitCharacter, ECollisionEnabled::Type NewType)
{
	if(IsValid(HitCharacter) && IsValid(HitCharacter->GetMesh()))
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(NewType);
	}
}

void ULagCompensationComponent::CacheBoxPositions(AEndlessBetrayalCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(!IsValid(HitCharacter)) return;

	for (const TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(IsValid(HitBoxPair.Value))
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			
			OutFramePackage.HitBoxInformationMap.Add(HitBoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(AEndlessBetrayalCharacter* HitCharacter, const FFramePackage& FrameToMoveAt)
{
	if(!IsValid(HitCharacter)) return;

	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(IsValid(HitBoxPair.Value))
		{
			HitBoxPair.Value->SetWorldLocation(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(AEndlessBetrayalCharacter* HitCharacter, const FFramePackage& FrameToMoveAt)
{
	if(!IsValid(HitCharacter)) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(IsValid(HitBoxPair.Value))
		{
			HitBoxPair.Value->SetWorldLocation(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FrameToMoveAt.HitBoxInformationMap[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickSaveFramePackage();
}

FShotgunServerSideRewindResults ULagCompensationComponent::ShotgunServerSideRewind(const TArray<AEndlessBetrayalCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (AEndlessBetrayalCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResults ULagCompensationComponent::ServerSideRewind(AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}


FFramePackage ULagCompensationComponent::GetFrameToCheck(AEndlessBetrayalCharacter* HitCharacter, float HitTime)
{
	if(!IsValid(HitCharacter) || !HitCharacter->GetLagCompensationComponent()
		|| HitCharacter->GetLagCompensationComponent()->FramesHistory.IsEmpty()
		|| !HitCharacter->GetLagCompensationComponent()->FramesHistory.GetHead()
		|| !HitCharacter->GetLagCompensationComponent()->FramesHistory.GetTail()) FServerSideRewindResults();

	//Frame history that belongs to the Character being hit
	const TDoubleLinkedList<FFramePackage>& HitCharacterFramesHistory = HitCharacter->GetLagCompensationComponent()->FramesHistory;
	const float OldestHistoryTime = HitCharacterFramesHistory.GetTail()->GetValue().Time;
	const float YoungestHistoryTime = HitCharacterFramesHistory.GetHead()->GetValue().Time;

	//Frame Package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	//(HitTime < OldestHistoryTime) -> HitTime si too far back  to do ServerSide Rewind
	if( HitTime < OldestHistoryTime) FServerSideRewindResults();

	if(HitTime == OldestHistoryTime)
	{
		FrameToCheck = HitCharacterFramesHistory.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
		
	if(HitTime >= YoungestHistoryTime)
	{
		FrameToCheck = HitCharacterFramesHistory.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	//TODO : To avoid loop if we've found the last FrameToCheck above
	
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = HitCharacterFramesHistory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = YoungerNode;

	while(OlderNode->GetValue().Time > HitTime) //Is Older still younger than HitTime
	{
		//March back until: OlderTime < HitTime < YoungerTime
		if(!OlderNode->GetNextNode()) break;
		OlderNode = OlderNode->GetNextNode();

		if(OlderNode->GetValue().Time > HitTime)
		{
			YoungerNode = OlderNode;
		}
	}

	//Slight chance of the following but here just in case
	if(OlderNode->GetValue().Time == HitTime)
	{
		FrameToCheck = OlderNode->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		//Interpolate Between Younger & Older
		FrameToCheck = InterpBetweenFrames(OlderNode->GetValue(), YoungerNode->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResults Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(IsValid(Character) && IsValid(HitCharacter) && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), Character->Controller, DamageCauser, UDamageType::StaticClass());
	}

}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<AEndlessBetrayalCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResults ConfirmHits = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	
	for (AEndlessBetrayalCharacter* HitCharacter : HitCharacters)
	{
		if(!IsValid(Character) || !IsValid(Character->GetEquippedWeapon()) || !IsValid(Character)) continue;
		float TotalShotDamage = 0.0f;
		
		if(ConfirmHits.HeadShotsMap.Contains(HitCharacter))
		{
			//TODO : Replace GetDamage() call by a future GetHeadShotDamage()
			TotalShotDamage += ConfirmHits.HeadShotsMap[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();
		}
		if(ConfirmHits.BodyShots.Contains(HitCharacter))
		{
			TotalShotDamage += ConfirmHits.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();
		}

		UGameplayStatics::ApplyDamage(HitCharacter, TotalShotDamage, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}