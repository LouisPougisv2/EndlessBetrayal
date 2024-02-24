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

	FLinearColor CrosshairColor = FLinearColor::White;
};

//Note: Crosshair is in the Weapon class as Weapons can have their own crosshair textures
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDTexture(const FHUDTextures& InHUDTextures) { HUDTextures = InHUDTextures; }

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcement Widget")
	TSubclassOf<UUserWidget> AnnouncementWidgetClass;

	UPROPERTY()
	class UAnnouncementUserWidget* AnnouncementWidget;

	void AddCharacterOverlay();
	void AddAnnouncementWidget();
	void HideKillDeathMessages();

	void AddEliminationAnnouncement(FString Attacker, FString Victim);
	
protected:

	virtual void BeginPlay() override;
	
private:

	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 10.0f;
	
	FHUDTextures HUDTextures;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UEliminationAnnouncementWidget> EliminationAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float EliminationAnnouncementLifeTime  = 2.5f;

	UPROPERTY()
	TArray<UEliminationAnnouncementWidget*> EliminationMessages;
	
	UFUNCTION()
	void EliminationAnnouncementTimerFinished(UEliminationAnnouncementWidget* MsgToRemove);
};
