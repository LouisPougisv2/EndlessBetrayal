// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "EndlessBetrayal/GameState/EndlessBetrayalGameState.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bIsTeamsMatch = true;
}

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

void ATeamsGameMode::OnPlayerEliminated(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController)
{
	Super::OnPlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

	AEndlessBetrayalGameState* EndlessBetrayalGameState = Cast<AEndlessBetrayalGameState>(UGameplayStatics::GetGameState(this));
	const AEndlessBetrayalPlayerState* AttackerPlayerState = Cast<AEndlessBetrayalPlayerState>(AttackerController->PlayerState);

	if(IsValid(EndlessBetrayalGameState) && IsValid(AttackerPlayerState))
	{
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			EndlessBetrayalGameState->IncrementBlueTeamScore();
		}
		
		if(AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			EndlessBetrayalGameState->IncrementRedTeamScore();
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* AttackerController, AController* VictimController, float Damages)
{
	const AEndlessBetrayalPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<AEndlessBetrayalPlayerState>();
	const AEndlessBetrayalPlayerState* VictimPlayerState = VictimController->GetPlayerState<AEndlessBetrayalPlayerState>();
	if(!IsValid(AttackerPlayerState) || !IsValid(VictimPlayerState)) return Damages;

	//Self Damage
	if(AttackerPlayerState == VictimPlayerState)
	{
		return Super::CalculateDamage(AttackerController, VictimController, Damages);
	}

	//Preventing Friendly Fire
	if(AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.0f;
	}

	//Shooting another player on another team
	return Super::CalculateDamage(AttackerController, VictimController, Damages);
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
