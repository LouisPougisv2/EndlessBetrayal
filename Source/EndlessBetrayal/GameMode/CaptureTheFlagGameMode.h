// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:

	virtual void OnPlayerEliminated(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone);
};
