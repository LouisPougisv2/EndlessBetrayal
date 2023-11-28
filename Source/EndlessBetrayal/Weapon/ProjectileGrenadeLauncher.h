// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AProjectileGrenadeLauncher : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileGrenadeLauncher();

protected:

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:

	UPROPERTY(EditAnywhere)
	USoundCue* BouncingCue;
	
};
