// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Components/BoxComponent.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& InFramePackage)
{
	Character = !IsValid(Character) ? Cast<AEndlessBetrayalCharacter>(GetOwner()) : Character;
	if(IsValid(Character) && !Character->HitCollisionBoxes.IsEmpty())
	{
		//As we're calling this function only on the server, GetWorld()->GetTimeSeconds() gives the official server time
		InFramePackage.Time = GetWorld()->GetTimeSeconds();
		
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

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
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

		//TODO : Temporary displaying the Frame Package, to remove soon
		ShowFramePackage(ThisFramePackage, FColor::Red);
	}
}

void ULagCompensationComponent::ServerSideRewind(AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	if(!IsValid(HitCharacter) || !HitCharacter->GetLagCompensationComponent()
		|| HitCharacter->GetLagCompensationComponent()->FramesHistory.IsEmpty()
		|| !HitCharacter->GetLagCompensationComponent()->FramesHistory.GetHead()
		|| !HitCharacter->GetLagCompensationComponent()->FramesHistory.GetTail()) return;

	//Frame history that belongs to the Character being hit
	const TDoubleLinkedList<FFramePackage>& HitCharacterFramesHistory = HitCharacter->GetLagCompensationComponent()->FramesHistory;
	const float OldestHistoryTime = HitCharacterFramesHistory.GetTail()->GetValue().Time;
	const float YoungestHistoryTime = HitCharacterFramesHistory.GetHead()->GetValue().Time;

	//Frame Package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	//(HitTime < OldestHistoryTime) -> HitTime si too far back  to do ServerSide Rewind
	if( HitTime < OldestHistoryTime) return;

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
	}

}

