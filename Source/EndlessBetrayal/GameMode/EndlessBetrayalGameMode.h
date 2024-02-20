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
	virtual void Logout(AController* Exiting) override;
	void OnPlayerLeftGame(AEndlessBetrayalPlayerState* PlayerLeavingTheGame);
	virtual float CalculateDamage(AController* AttackerController, AController* VictimController, float Damages);

	FORCEINLINE float GetCountdownTime() const { return CountDownTime; }

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 190.0f;

	//Time that it takes from launching the game to entering EndlessBetrayal map
	UPROPERTY()
	float LevelStartingTime = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 30.0f;

protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	
	UPROPERTY()
	float CountDownTime = 0.0f;

};
