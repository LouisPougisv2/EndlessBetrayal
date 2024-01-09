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

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);

	ShowFramePackage(FramePackage, FColor::Orange);
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
		DrawDebugBox(GetWorld(), HitBox.Value.Location, HitBox.Value.BoxExtent, FQuat(HitBox.Value.Rotation), Color, true);
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

