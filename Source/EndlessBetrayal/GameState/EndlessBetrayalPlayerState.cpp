// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalPlayerState.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"

void AEndlessBetrayalPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	EndlessBetrayalCharacter = EndlessBetrayalCharacter == nullptr ? Cast<AEndlessBetrayalCharacter>(GetPawn()) : EndlessBetrayalCharacter;
	if(EndlessBetrayalCharacter)
	{
		EndlessBetrayalPlayerController = EndlessBetrayalPlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(EndlessBetrayalCharacter->Controller) : EndlessBetrayalPlayerController;
		if(EndlessBetrayalPlayerController)
		{
			EndlessBetrayalPlayerController->UpdateScoreHUD(GetScore());
		}
	}
	
}

void AEndlessBetrayalPlayerState::AddToScore(float NewScore)
{
	SetScore(GetScore() + NewScore);

	EndlessBetrayalCharacter = EndlessBetrayalCharacter == nullptr ? Cast<AEndlessBetrayalCharacter>(GetPawn()) : EndlessBetrayalCharacter;
	if(EndlessBetrayalCharacter)
	{
		EndlessBetrayalPlayerController = EndlessBetrayalPlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(EndlessBetrayalCharacter->Controller) : EndlessBetrayalPlayerController;
		if(EndlessBetrayalPlayerController)
		{
			EndlessBetrayalPlayerController->UpdateScoreHUD(GetScore());
		}
	}
}
