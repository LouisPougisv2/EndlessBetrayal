// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	StartSpawnPickupTimer(SpawnedPickup);
}

void APickupSpawnPoint::SpawnPickup()
{
	if(!PickupClasses.IsEmpty())
	{
		int32 RandomIndex = FMath::RandRange(0, PickupClasses.Num() - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[RandomIndex], GetActorTransform());

		if(IsValid(SpawnedPickup))
		{
			SpawnedPickup->OnDestroyed.AddUniqueDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTimeRate = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorld()->GetTimerManager().SetTimer(SpawnPickupTimer, this,  &APickupSpawnPoint::OnSpawnPickupTimerFinished, SpawnTimeRate);
}

void APickupSpawnPoint::OnSpawnPickupTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

