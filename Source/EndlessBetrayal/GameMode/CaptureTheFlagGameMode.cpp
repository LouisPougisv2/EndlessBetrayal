// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "EndlessBetrayal/CaptureTheFlag/FlagZone.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalGameState.h"
#include "EndlessBetrayal/Weapon/Flag.h"

class AEndlessBetrayalGameState;

void ACaptureTheFlagGameMode::OnPlayerEliminated(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController)
{
	AEndlessBetrayalGameMode::OnPlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
	if(!IsValid(Flag) || !IsValid(FlagZone)) return;

	const bool bIsValidCapture = Flag->GetTeam() != FlagZone->GetFlagZoneTeam();
	AEndlessBetrayalGameState* EndlessBetrayalGameState = GetGameState<AEndlessBetrayalGameState>();

	if(bIsValidCapture && EndlessBetrayalGameState)
	{
		if(FlagZone->GetFlagZoneTeam() == ETeam::ET_BlueTeam)
		{
			EndlessBetrayalGameState->IncrementBlueTeamScore();
		}

		if(FlagZone->GetFlagZoneTeam() == ETeam::ET_RedTeam)
		{
			EndlessBetrayalGameState->IncrementRedTeamScore();
		}
	}
	
}
