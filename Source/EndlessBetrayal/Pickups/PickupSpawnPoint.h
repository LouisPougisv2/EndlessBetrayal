// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class ENDLESSBETRAYAL_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	void SpawnPickup();
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup = nullptr;

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

	//Delegate ALWAYS need to be UFUNCTION()
	UFUNCTION()
	void OnSpawnPickupTimerFinished();
private:

	FTimerHandle SpawnPickupTimer;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 10.0f;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 20.0f;

};
