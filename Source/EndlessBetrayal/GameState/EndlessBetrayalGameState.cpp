// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalGameState.h"
#include "EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Net/UnrealNetwork.h"

void AEndlessBetrayalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEndlessBetrayalGameState, TopScoringPlayers);
	DOREPLIFETIME(AEndlessBetrayalGameState, RedTeamScore);
	DOREPLIFETIME(AEndlessBetrayalGameState, BlueTeamScore);
	
}

void AEndlessBetrayalGameState::UpdateTopScore(AEndlessBetrayalPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)	//We have tie
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if(ScoringPlayer->GetScore() > TopScore)	//the Scoring player sets a new top score
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AEndlessBetrayalGameState::IncrementBlueTeamScore()
{
	++BlueTeamScore;

	AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController = Cast<AEndlessBetrayalPlayerController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateHUDBlueTeamScore(BlueTeamScore);
	}
}

void AEndlessBetrayalGameState::IncrementRedTeamScore()
{
	++RedTeamScore;
	
	AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController = Cast<AEndlessBetrayalPlayerController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateHUDRedTeamScore(RedTeamScore);
	}
}

void AEndlessBetrayalGameState::OnRep_RedTeamScore()
{
	AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController = Cast<AEndlessBetrayalPlayerController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateHUDRedTeamScore(RedTeamScore);
	}
}

void AEndlessBetrayalGameState::OnRep_BlueTeamScore()
{
	AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController = Cast<AEndlessBetrayalPlayerController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateHUDBlueTeamScore(BlueTeamScore);
	}
}
