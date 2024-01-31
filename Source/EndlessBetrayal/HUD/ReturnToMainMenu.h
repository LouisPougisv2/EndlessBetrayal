// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	void MenuSetUp();
	void MenuTearDown();

protected:
	
	virtual bool Initialize() override;

	void BindDelegates();

	void UnBindDelegates();
	
	UFUNCTION()
	void OnDestroyedSession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

private:

	UPROPERTY(meta =(BindWidget))
	class UButton* ReturnButton;

	UFUNCTION()
	void OnReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

};
