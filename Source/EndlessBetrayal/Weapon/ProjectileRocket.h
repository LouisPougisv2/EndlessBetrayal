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

#if WITH_EDITOR
	//Allows us to spread any changes happening in BP
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:

	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void UpdateRocketEffects();

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
	
private:

	UPROPERTY(EditAnywhere)
	USoundCue* RocketSoundCue;

	UPROPERTY()
	UAudioComponent* RocketSoundComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* RocketSoundAttenuation;
	
};
