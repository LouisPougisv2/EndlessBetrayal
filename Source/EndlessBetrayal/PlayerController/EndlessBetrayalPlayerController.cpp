// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/HUD/CharacterOverlay.h"
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"



void AEndlessBetrayalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
}

void AEndlessBetrayalPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
}

void AEndlessBetrayalPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEndlessBetrayalPlayerController, MatchState);
}

void AEndlessBetrayalPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	const AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(InPawn);
	UpdateHealthHUD(EndlessBetrayalCharacter->GetHealth(), EndlessBetrayalCharacter->GetMaxHealth());

	if(IsValid(EndlessBetrayalHUD) && IsValid(EndlessBetrayalHUD->CharacterOverlay))
	{
		HideMessagesOnScreenHUD();
	}
}

void AEndlessBetrayalPlayerController::UpdateHealthHUD(float NewHealth, float MaxHealth)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->HealthBar && EndlessBetrayalHUD->CharacterOverlay->HealthText;
	if(bIsHUDVariableFullyValid)
	{
		const float HealthPercent = NewHealth / MaxHealth;
		EndlessBetrayalHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(NewHealth), FMath::CeilToInt(MaxHealth));
		EndlessBetrayalHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bShouldInitializeCharacterOverlay = true;
		HUDHealth = NewHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AEndlessBetrayalPlayerController::UpdateScoreHUD(float NewScore)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->HealthBar && EndlessBetrayalHUD->CharacterOverlay->ScoreValue;
	if(bIsHUDVariableFullyValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(NewScore));
		EndlessBetrayalHUD->CharacterOverlay->ScoreValue->SetText(FText::FromString(ScoreText));

		if(EndlessBetrayalHUD->CharacterOverlay->KillText)
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AEndlessBetrayalPlayerController::HideMessagesOnScreenHUD, 2.0f);
			EndlessBetrayalHUD->CharacterOverlay->KillText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		bShouldInitializeCharacterOverlay = true;
		HUDScore = NewScore;
	}
}

void AEndlessBetrayalPlayerController::UpdateDeathsHUD(int NewDeath)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->HealthBar && EndlessBetrayalHUD->CharacterOverlay->DeathsValue;
	if(bIsHUDVariableFullyValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), NewDeath);
		EndlessBetrayalHUD->CharacterOverlay->DeathsValue->SetText(FText::FromString(ScoreText));

		if(EndlessBetrayalHUD->CharacterOverlay->DeathText)
		{
			EndlessBetrayalHUD->CharacterOverlay->DeathText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		bShouldInitializeCharacterOverlay = true;
		HUDDeaths = NewDeath;
	}
}

void AEndlessBetrayalPlayerController::UpdateHUDMatchCountdown(float CountdownTime)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText;
	if(bIsHUDVariableFullyValid)
	{
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		const FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountDownText));
	}
}

void AEndlessBetrayalPlayerController::HideMessagesOnScreenHUD()
{
	if(EndlessBetrayalHUD)
	{
		EndlessBetrayalHUD->HideKillDeathMessages();
	}
}

void AEndlessBetrayalPlayerController::UpdateWeaponAmmo(int32 NewAmmo)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->HealthBar && EndlessBetrayalHUD->CharacterOverlay->WeaponAmmoAmount;
	if(bIsHUDVariableFullyValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), NewAmmo);
		EndlessBetrayalHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AEndlessBetrayalPlayerController::UpdateWeaponCarriedAmmo(int32 NewAmmo)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->HealthBar && EndlessBetrayalHUD->CharacterOverlay->CarriedAmmoAmount;
	if(bIsHUDVariableFullyValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), NewAmmo);
		EndlessBetrayalHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AEndlessBetrayalPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AEndlessBetrayalPlayerController::DisplayCharacterOverlay()
{
	if(MatchState == MatchState::InProgress)
	{
		EndlessBetrayalHUD = !IsValid(EndlessBetrayalHUD) ? EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD()) : EndlessBetrayalHUD;
		if(IsValid(EndlessBetrayalHUD))
		{
			//HUD will only be displayed when the match is In Progress
			EndlessBetrayalHUD->AddCharacterOverlay();
		}
	}
}

void AEndlessBetrayalPlayerController::OnMatchStateSet(FName NewMatchState)
{
	MatchState = NewMatchState;

	DisplayCharacterOverlay();
}


void AEndlessBetrayalPlayerController::OnRep_MatchState()
{
	DisplayCharacterOverlay();
}

void AEndlessBetrayalPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

void AEndlessBetrayalPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	if(CountDownInt != SecondsLeft)
	{
		UpdateHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountDownInt = SecondsLeft;
}

void AEndlessBetrayalPlayerController::PollInit()
{
	if(!IsValid(CharacterOverlay))
	{
		if(IsValid(EndlessBetrayalHUD) && IsValid(EndlessBetrayalHUD->CharacterOverlay))
		{
			CharacterOverlay = EndlessBetrayalHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				UpdateHealthHUD(HUDHealth, HUDMaxHealth);
				UpdateScoreHUD(HUDScore);
				UpdateDeathsHUD(HUDDeaths);
			}
		}
	}
}

void AEndlessBetrayalPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AEndlessBetrayalPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	//Approximation of Current Time on Server
	const float CurrentServerTime = TimeServerReceivedClientRequest + ( RoundTripTime / 2);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AEndlessBetrayalPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;	
}