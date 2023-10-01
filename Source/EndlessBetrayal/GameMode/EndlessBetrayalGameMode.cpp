// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalGameMode.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AEndlessBetrayalGameMode::OnPlayerEliminated(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController)
{
	if(!ensureAlways(IsValid(EliminatedCharacter)) || !ensureAlways(IsValid(VictimController)) || !ensureAlways(IsValid(AttackerController))) return;

	AEndlessBetrayalPlayerState* AttackerPlayerState = Cast<AEndlessBetrayalPlayerState>(AttackerController->PlayerState);
	AEndlessBetrayalPlayerState* VictimPlayerState = Cast<AEndlessBetrayalPlayerState>(VictimController->PlayerState);

	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.0f);
	}
	
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToKills(1);
	}
	EliminatedCharacter->OnPlayerEliminated();
}

void AEndlessBetrayalGameMode::RequestRespawn(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* EliminatedController)
{
	if(IsValid(EliminatedCharacter))
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if(IsValid(EliminatedController))
	{
		TArray<AActor*> PlayerStartActors;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartActors);
		const int32 IndexSelectedPlayerStart = FMath::RandRange(0, PlayerStartActors.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStartActors[IndexSelectedPlayerStart]);
	}
}