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

	AEndlessBetrayalGameMode();
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnPlayerEliminated(class AEndlessBetrayalCharacter* EliminatedCharacter, class AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController);
	virtual void RequestRespawn(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.0f;

	//Time that it takes from launching the game to entering EndlessBetrayal map
	float LevelStartingTime = 0.0f;

protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	
	UPROPERTY()
	float CountDownTime = 0.0f;

};
