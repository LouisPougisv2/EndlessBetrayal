// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* ScoreValue;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* DeathsValue;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* BlueTeamScore;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* RedTeamScore;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* TeamScoreSlash;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UImage* HighPingImage;

	//For WidgetAnimation, the meta BindWidgetAnim only works if the variable is Transient !! (not serialized to disk)
	UPROPERTY(meta = (BindWidgetAnim), Transient, BlueprintReadOnly, Category = "Player Stats")
	UWidgetAnimation* HighPingAnimation;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* KillText;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* DeathText;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* GrenadeAmount;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	UTextBlock* MatchCountdownText;
};
