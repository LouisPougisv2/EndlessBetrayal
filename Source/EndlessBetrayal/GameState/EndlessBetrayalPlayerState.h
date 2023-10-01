// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	virtual void OnRep_Score() override;

	//Should be called only on the server, as the latter is in charge of the score
	void AddToScore(float NewScore);

private:

	UPROPERTY()
	class AEndlessBetrayalCharacter* EndlessBetrayalCharacter;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController;
};
