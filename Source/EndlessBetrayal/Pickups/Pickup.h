// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class ENDLESSBETRAYAL_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	

	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BindOverlapDelegate();
	
	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	bool ShouldPickupRotate = true;
	
	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	float BaseRotatingRate = 45.0f;
	
private:

	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	class UStaticMeshComponent* PickupMesh;
	
	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	class USphereComponent* CollisionSphere;

	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	class USoundCue* PickupSound;
	
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	FTimerHandle BindOverlapTimer;

	float BindOverlapTime = 0.3f;
};
