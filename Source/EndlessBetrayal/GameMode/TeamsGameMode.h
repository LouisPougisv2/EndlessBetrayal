// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EndlessBetrayalGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API ATeamsGameMode : public AEndlessBetrayalGameMode
{
	GENERATED_BODY()

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* AttackerController, AController* VictimController, float Damages) override;
	
protected:

	virtual void HandleMatchHasStarted() override;
};
