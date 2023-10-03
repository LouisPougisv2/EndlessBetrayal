// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EndlessBetrayalPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void UpdateHealthHUD(float NewHealth, float MaxHealth);
	void UpdateScoreHUD(float NewScore);
	void UpdateDeathsHUD(int32 NewDeath);
	void HideMessagesOnScreenHUD();

	/**
	 * Ammo
	 */
	void UpdateWeaponAmmo(int32 NewAmmo);

	
	void OnPossess(APawn* InPawn) override;
protected:

	virtual void BeginPlay() override;
	
private:

	class AEndlessBetrayalHUD* EndlessBetrayalHUD;
	
};
