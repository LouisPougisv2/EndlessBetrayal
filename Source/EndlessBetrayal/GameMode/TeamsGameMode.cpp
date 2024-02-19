// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "EndlessBetrayal/GameState/EndlessBetrayalGameState.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AEndlessBetrayalGameState* EndlessBetrayalGameState = Cast<AEndlessBetrayalGameState>(UGameplayStatics::GetGameState(this));
	if(IsValid(EndlessBetrayalGameState))
	{
		AEndlessBetrayalPlayerState* EndlessBetrayalPlayerState = NewPlayer->GetPlayerState<AEndlessBetrayalPlayerState>();
		if(IsValid(EndlessBetrayalPlayerState) && EndlessBetrayalPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if(EndlessBetrayalGameState->BlueTeam.Num() >= EndlessBetrayalGameState->RedTeam.Num())
			{
				EndlessBetrayalGameState->RedTeam.AddUnique(EndlessBetrayalPlayerState);
				EndlessBetrayalPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				EndlessBetrayalGameState->BlueTeam.AddUnique(EndlessBetrayalPlayerState);
				EndlessBetrayalPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
		
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AEndlessBetrayalGameState* EndlessBetrayalGameState = Cast<AEndlessBetrayalGameState>(UGameplayStatics::GetGameState(this));
	AEndlessBetrayalPlayerState* EndlessBetrayalPlayerState = Exiting->GetPlayerState<AEndlessBetrayalPlayerState>();
	if(IsValid(EndlessBetrayalGameState) && IsValid(EndlessBetrayalPlayerState))
	{
		if(EndlessBetrayalGameState->RedTeam.Contains(EndlessBetrayalPlayerState))
		{
			EndlessBetrayalGameState->RedTeam.Remove(EndlessBetrayalPlayerState);
		}
		if(EndlessBetrayalGameState->BlueTeam.Contains(EndlessBetrayalPlayerState))
		{
			EndlessBetrayalGameState->BlueTeam.Remove(EndlessBetrayalPlayerState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AEndlessBetrayalGameState* EndlessBetrayalGameState = Cast<AEndlessBetrayalGameState>(UGameplayStatics::GetGameState(this));
	if(IsValid(EndlessBetrayalGameState))
	{
		for (TObjectPtr<APlayerState> Player : EndlessBetrayalGameState->PlayerArray)
		{
			AEndlessBetrayalPlayerState* EndlessBetrayalPlayerState = Cast<AEndlessBetrayalPlayerState>(Player.Get());
			if(IsValid(EndlessBetrayalPlayerState) && EndlessBetrayalPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(EndlessBetrayalGameState->BlueTeam.Num() >= EndlessBetrayalGameState->RedTeam.Num())
				{
					EndlessBetrayalGameState->RedTeam.AddUnique(EndlessBetrayalPlayerState);
					EndlessBetrayalPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					EndlessBetrayalGameState->BlueTeam.AddUnique(EndlessBetrayalPlayerState);
					EndlessBetrayalPlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
