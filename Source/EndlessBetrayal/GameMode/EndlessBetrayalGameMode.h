// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EndlessBetrayalGameMode.generated.h"

namespace MatchState
{
	//Match duration has been reached. Display winner and begin cooldown timer.
	extern ENDLESSBETRAYAL_API const FName Cooldown;
}
/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AEndlessBetrayalGameMode();
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnPlayerEliminated(class AEndlessBetrayalCharacter* EliminatedCharacter, class AEndlessBetrayalPlayerController* VictimController, AEndlessBetrayalPlayerController* AttackerController);
	virtual void RequestRespawn(AEndlessBetrayalCharacter* EliminatedCharacter, AEndlessBetrayalPlayerController* EliminatedController);

	FORCEINLINE float GetCountdownTime() const { return CountDownTime; }

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 20.0f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 180.0f;

	//Time that it takes from launching the game to entering EndlessBetrayal map
	UPROPERTY(EditDefaultsOnly)
	float LevelStartingTime = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;

protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	
	UPROPERTY()
	float CountDownTime = 0.0f;

};
