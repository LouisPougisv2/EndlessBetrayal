// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class ENDLESSBETRAYAL_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	void SetDamage(const float InDamage) { Damage = InDamage; }
	void SetHeadShotDamage(const float InHeadShotDamage) { HeadShotDamage = InHeadShotDamage; }

	/**
	* Used with Server Side Rewind
	*/

	UPROPERTY()
	bool bUseServerSideRewind = false;

	UPROPERTY()
	FVector_NetQuantize TraceStart;

	UPROPERTY()
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.0f;
	
protected:
	
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyOnTimerFinished();

	void ExplodeDamage();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();

	//Only set this for Rocket and Damages
	UPROPERTY(EditAnywhere)
	float Damage = 15.0f;

	//Doesn't matter for Rocket & Grenades
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 30.0f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* SmokeTrailSystem;

	UPROPERTY()
	class UNiagaraComponent* SmokeTrailComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
private:

	UPROPERTY(EditAnywhere, Category = "Zone Damage")
	float MinimumDamage = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Zone Damage")
	float DamageInnerRadius = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Zone Damage")
	float DamageOuterRadius = 500.0f;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;

};
