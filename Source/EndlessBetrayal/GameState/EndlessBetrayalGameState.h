// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "EndlessBetrayalGameState.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(class AEndlessBetrayalPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AEndlessBetrayalPlayerState*> TopScoringPlayers;

private:

	UPROPERTY()
	float TopScore = 0.0f;
	
};
