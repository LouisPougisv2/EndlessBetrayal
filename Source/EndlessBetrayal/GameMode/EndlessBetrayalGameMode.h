// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EndlessBetrayalGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	virtual void OnPlayerEliminated(class AEndlessBetrayalCharacter* EliminatedCharacter, class AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController);
};
