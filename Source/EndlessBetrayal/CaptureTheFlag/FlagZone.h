// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/Team.h"
#include "GameFramework/Actor.h"
#include "FlagZone.generated.h"

UCLASS()
class ENDLESSBETRAYAL_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	

	AFlagZone();

	FORCEINLINE ETeam  GetFlagZoneTeam() const { return FlagZoneTeam; }
protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	UPROPERTY(EditAnywhere)
	class USphereComponent* FlagZoneSphere;

	UPROPERTY(EditAnywhere)
	ETeam FlagZoneTeam = ETeam::ET_NoTeam;
};
