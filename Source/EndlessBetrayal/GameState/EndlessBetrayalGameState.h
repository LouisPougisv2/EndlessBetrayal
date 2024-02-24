// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/Team.h"
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

	void IncrementBlueTeamScore();
	void IncrementRedTeamScore();
	
	UPROPERTY(Replicated)
	TArray<AEndlessBetrayalPlayerState*> TopScoringPlayers;

	/*
	* Teams
	*/

	TArray<AEndlessBetrayalPlayerState*> RedTeam;
	TArray<AEndlessBetrayalPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.0f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:

	UPROPERTY()
	float TopScore = 0.0f;
	
};
