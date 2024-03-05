// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* EndlessBetrayalGameInstance = GetGameInstance();
	if(IsValid(EndlessBetrayalGameInstance))
	{
		UMultiplayerSessionsSubsystem* MultiplayerSubsystem = EndlessBetrayalGameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(MultiplayerSubsystem);

		if (NumberOfPlayers == MultiplayerSubsystem->GetDesiredNumPublicConnections())
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				const FString MatchType = MultiplayerSubsystem->GetDesiredMatchType();

				if(MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/EndlessBetrayalMap?listen"));
				}
				else if(MatchType == "TeamsMatch")
				{
					World->ServerTravel(FString("/Game/Maps/TeamsMap?listen"));
				}
				else if(MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlagMap?listen"));
				}
			}
		}
	}
}