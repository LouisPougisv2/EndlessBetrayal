// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)			//so we can use this enum as a type in blueprint
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName= "DefaultMax")
};

UCLASS()
class ENDLESSBETRAYAL_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ABulletCasing> BulletCasingClass;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairRight;

	/**
	*	Zoomed FOV while Zooming
	**/

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomInterpSpeed = 20.0f;
	
	
public:	

	void SetWeaponState(EWeaponState NewState);
	FORCEINLINE USphereComponent* GetAreaSphere() const	{ return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UTexture2D* GetCrosshairCenter() const { return CrosshairCenter; }
	FORCEINLINE UTexture2D* GetCrosshairTop() const { return CrosshairTop; }
	FORCEINLINE UTexture2D* GetCrosshairBottom() const { return CrosshairBottom; }
	FORCEINLINE UTexture2D* GetCrosshairLeft() const { return CrosshairLeft; }
	FORCEINLINE UTexture2D* GetCrosshairRight() const { return CrosshairRight; }

	FORCEINLINE float GetZoomedFOV() { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterSpeed() const { return ZoomInterpSpeed; }
};
