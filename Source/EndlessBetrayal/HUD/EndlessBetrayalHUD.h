// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EndlessBetrayalHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDTextures
{
	GENERATED_BODY()

	UPROPERTY()
	class UTexture2D* CrosshairCenter = nullptr;

	UPROPERTY()
	UTexture2D* CrosshairLeft = nullptr;

	UPROPERTY()
	UTexture2D* CrosshairTop = nullptr;

	UPROPERTY()
	UTexture2D* CrosshairRight = nullptr;

	UPROPERTY()
	UTexture2D* CrosshairBottom = nullptr;

	float CrosshairSpreadFactor = 0.0f;
};

//Note: Crosshair is in the Weapon class as Weapons can have their own crosshair textures
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDTexture(const FHUDTextures& InHUDTextures) { HUDTextures = InHUDTextures; }
private:

	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 10.0f;
	
	FHUDTextures HUDTextures;
	
};
