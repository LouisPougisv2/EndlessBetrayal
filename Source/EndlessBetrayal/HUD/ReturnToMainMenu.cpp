// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::MenuSetUp()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	APlayerController* OwningController = GetOwningPlayer();
	if(IsValid(OwningController))
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		OwningController->SetInputMode(InputModeData);
		OwningController->SetShowMouseCursor(true);
	}

	BindDelegates();
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	APlayerController* OwningController = GetOwningPlayer();
	if(IsValid(OwningController))
	{
		FInputModeGameOnly InputModeData;
		OwningController->SetInputMode(InputModeData);
		OwningController->SetShowMouseCursor(false);
	}

	UnBindDelegates();
}

bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize()) return false;
	
	return true;
}

void UReturnToMainMenu::BindDelegates()
{
	if(ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::OnReturnButtonClicked);
	}
	
	const UWorld* World = GetWorld();
	if(IsValid(World))
	{
		MultiplayerSessionsSubsystem = World->GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if(IsValid(MultiplayerSessionsSubsystem))
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroyedSession);
		}
	}
}

void UReturnToMainMenu::UnBindDelegates()
{
	if(ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::OnReturnButtonClicked);
	}
	
	if(IsValid(MultiplayerSessionsSubsystem))
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroyedSession);
	}
}

void UReturnToMainMenu::OnDestroyedSession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}
	
	UWorld* World = GetWorld();
	if(IsValid(World))
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if(GameMode) //We know we're on the server
		{
			GameMode->ReturnToMainMenuHost();
		}
		else //We're on the client
		{
			APlayerController* OwningController = GetOwningPlayer();
			if(IsValid(OwningController))
			{
				//Using the following function signature instead of "ClientReturnToMainMenu" as UE tells us it is a "better" version
				OwningController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if(IsValid(MultiplayerSessionsSubsystem))
	{
		MultiplayerSessionsSubsystem->DestroySession();	//Will disconnect player from the session
	}
}

void UReturnToMainMenu::OnReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	APlayerController* PlayerController = GetOwningPlayer();
	if(IsValid(PlayerController))
	{
		AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(PlayerController->GetPawn());
		if(IsValid(EndlessBetrayalCharacter))
		{
			EndlessBetrayalCharacter->ServerLeaveGame();
			EndlessBetrayalCharacter->OnPlayerLeftGameDelegate.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
		}
		else
		{
			ReturnButton->SetIsEnabled(true);
		}
	}
}
