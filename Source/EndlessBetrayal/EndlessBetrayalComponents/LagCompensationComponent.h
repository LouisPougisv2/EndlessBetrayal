// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//Contains the barebones information we need for a given box
USTRUCT()
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
	
};


//Struct that stores the information about all the HitBoxes for the character, for a given frame
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInformationMap;
	
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENDLESSBETRAYAL_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	friend class AEndlessBetrayalCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	AEndlessBetrayalCharacter* Character;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* PlayerController;
	
};
