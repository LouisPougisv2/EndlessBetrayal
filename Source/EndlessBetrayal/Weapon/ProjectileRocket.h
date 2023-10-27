// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileRocket();
	virtual void Destroyed() override;
	
protected:

	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void UpdateRocketEffects();
	void DestroyOnTimerFinished();

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

	
private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* SmokeTrailSystem;

	UPROPERTY()
	class UNiagaraComponent* SmokeTrailComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* RocketSoundCue;

	UPROPERTY()
	UAudioComponent* RocketSoundComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* RocketSoundAttenuation;
	
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;

	UPROPERTY(EditAnywhere)
	float MinimumDamage = 25.0f;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.0f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.0f;
	
};
