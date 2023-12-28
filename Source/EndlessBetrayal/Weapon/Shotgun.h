// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets);

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
	
};
