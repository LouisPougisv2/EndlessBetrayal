// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;

protected:

	virtual void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult);

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;
	
private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* BeamSystemComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
