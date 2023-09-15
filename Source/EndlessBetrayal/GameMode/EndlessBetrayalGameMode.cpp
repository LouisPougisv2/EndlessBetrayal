// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalGameMode.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"

void AEndlessBetrayalGameMode::OnPlayerEliminated(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController)
{
	if(!ensureAlways(IsValid(EliminatedCharacter)) || !ensureAlways(IsValid(VictimController)) || !ensureAlways(IsValid(AttackerController))) return;

	EliminatedCharacter->OnPlayerEliminated();
}
