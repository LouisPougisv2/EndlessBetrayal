// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalGameMode.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"


namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}


AEndlessBetrayalGameMode::AEndlessBetrayalGameMode()
{
	//Game mode will stay in waiting to start state and will stay in this state until we manually call StartMatch()
	//In the meantime, players will be able to "fly" through the level until StartMatch will give them a Pawn to control
	bDelayedStart = true;
}

void AEndlessBetrayalGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountDownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountDownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	}
}

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

void AEndlessBetrayalGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AEndlessBetrayalGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController = Cast<AEndlessBetrayalPlayerController>(*It);
		if(IsValid(EndlessBetrayalPlayerController))
		{
			EndlessBetrayalPlayerController->OnMatchStateSet(MatchState);
		}
	}
}
