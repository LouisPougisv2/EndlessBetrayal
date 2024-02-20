// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "EndlessBetrayalPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Deaths();

	//Should be called only on the server, as the latter is in charge of the score
	void AddToScore(float NewScore);
	void AddToKills(float NewKill);

	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam InTeam);

private:

	UPROPERTY()
	class AEndlessBetrayalCharacter* EndlessBetrayalCharacter;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController;

	UPROPERTY(ReplicatedUsing=OnRep_Deaths)
	int32 DeathsCount;

	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;
};
