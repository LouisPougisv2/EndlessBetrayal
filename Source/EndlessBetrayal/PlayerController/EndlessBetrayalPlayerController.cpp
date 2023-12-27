// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalPlayerController.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/CombatComponent.h"
#include "EndlessBetrayal/GameMode/EndlessBetrayalGameMode.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalGameState.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/HUD/AnnouncementUserWidget.h"
#include "EndlessBetrayal/HUD/CharacterOverlay.h"
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"



void AEndlessBetrayalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();
}

void AEndlessBetrayalPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
}

void AEndlessBetrayalPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEndlessBetrayalPlayerController, MatchState);
}

void AEndlessBetrayalPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = CastChecked<AEndlessBetrayalCharacter>(InPawn);
	EndlessBetrayalCharacter->UpdateHUDAmmo();
	UpdateHealthHUD(EndlessBetrayalCharacter->GetHealth(), EndlessBetrayalCharacter->GetMaxHealth());
	UpdateShieldHUD(EndlessBetrayalCharacter->GetShield(), EndlessBetrayalCharacter->GetMaxShield());

	if(IsValid(EndlessBetrayalCharacter->GetCombatComponent()))
	{
		UpdateGrenadesAmmo(EndlessBetrayalCharacter->GetCombatComponent()->GetGrenadesAmount());
	}
	
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
		HUDHealth = NewHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AEndlessBetrayalPlayerController::UpdateShieldHUD(float NewShieldValue, float MaxShield)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->ShieldBar && EndlessBetrayalHUD->CharacterOverlay->ShieldText;
	if(bIsHUDVariableFullyValid)
	{
		const float ShieldPercent = NewShieldValue / MaxShield;
		EndlessBetrayalHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(NewShieldValue), FMath::CeilToInt(MaxShield));
		EndlessBetrayalHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		HUDShield = NewShieldValue;
		HUDMaxShield = MaxShield;
	}
}

void AEndlessBetrayalPlayerController::UpdateScoreHUD(float NewScore)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->ScoreValue;
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
		HUDScore = NewScore;
	}
}

void AEndlessBetrayalPlayerController::UpdateDeathsHUD(int NewDeath)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->DeathsValue;
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
		HUDDeaths = NewDeath;
	}
}

void AEndlessBetrayalPlayerController::UpdateHUDMatchCountdown(float InCountdownTime)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText;
	if(bIsHUDVariableFullyValid)
	{		
		if(InCountdownTime < 0.0f)
		{
			EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		const int32 Minutes = FMath::FloorToInt(InCountdownTime / 60);
		const int32 Seconds = InCountdownTime - Minutes * 60;
		const FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountDownText));

		if((InCountdownTime <= 30.0f) && (InCountdownTime > 0.0f))
		{
			EndlessBetrayalHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FLinearColor::Red);
		}
	}
}

void AEndlessBetrayalPlayerController::UpdateHUDAnnouncementCountDown(float InCountdownTime)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->AnnouncementWidget && EndlessBetrayalHUD->AnnouncementWidget->AnnouncementText;
	if(bIsHUDVariableFullyValid)
	{
		if(InCountdownTime < 0.0f)
		{
			EndlessBetrayalHUD->AnnouncementWidget->WarmUpTimeText->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(InCountdownTime / 60);
		const int32 Seconds = InCountdownTime - Minutes * 60;
		const FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		
		EndlessBetrayalHUD->AnnouncementWidget->WarmUpTimeText->SetText(FText::FromString(CountDownText));
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

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->WeaponAmmoAmount;
	if(bIsHUDVariableFullyValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), NewAmmo);
		EndlessBetrayalHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		HUDWeaponAmmo = NewAmmo;
	}
}

void AEndlessBetrayalPlayerController::UpdateWeaponCarriedAmmo(int32 NewAmmo)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->CarriedAmmoAmount;
	if(bIsHUDVariableFullyValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), NewAmmo);
		EndlessBetrayalHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		HUDCarriedAmmo = NewAmmo;
	}
}

void AEndlessBetrayalPlayerController::UpdateGrenadesAmmo(int32 Grenades)
{
	if(!IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	}

	const bool bIsHUDVariableFullyValid = IsValid(EndlessBetrayalHUD) && EndlessBetrayalHUD->CharacterOverlay && EndlessBetrayalHUD->CharacterOverlay->GrenadeAmount;
	if(bIsHUDVariableFullyValid)
	{
		FString CarriedGrenadeText = FString::Printf(TEXT("%d"), Grenades);
		EndlessBetrayalHUD->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(CarriedGrenadeText));
	}
}

void AEndlessBetrayalPlayerController::HandleCooldown()
{
	EndlessBetrayalHUD = !IsValid(EndlessBetrayalHUD) ? EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD()) : EndlessBetrayalHUD; 
	if(IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD->CharacterOverlay->RemoveFromParent();

		const bool bIsHUDValid = IsValid(EndlessBetrayalHUD->AnnouncementWidget) && IsValid(EndlessBetrayalHUD->AnnouncementWidget->AnnouncementText) && IsValid(EndlessBetrayalHUD->AnnouncementWidget->InfoText);
		if(bIsHUDValid)
		{
			EndlessBetrayalHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			EndlessBetrayalHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString("New Match Starts In :"));

			FString InfoTextString;
			AEndlessBetrayalGameState* EndlessBetrayalGameState = Cast<AEndlessBetrayalGameState>(UGameplayStatics::GetGameState(this));
			AEndlessBetrayalPlayerState* EndlessBetrayalPlayerState = GetPlayerState<AEndlessBetrayalPlayerState>();
			if(IsValid(EndlessBetrayalGameState) && EndlessBetrayalPlayerState)
			{
				const TArray<AEndlessBetrayalPlayerState*>& TopPlayers = EndlessBetrayalGameState->TopScoringPlayers;
				if(TopPlayers.IsEmpty())	//No winners
				{
					InfoTextString = FString("There is no winners");
				}
				else if ((TopPlayers.Num() == 1) && (TopPlayers[0] == EndlessBetrayalPlayerState) )
				{
					InfoTextString = FString("You are the winner!");
				}
				else if(TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner : \n %s"), *TopPlayers[0]->GetPlayerName());
				}
				else
				{
					InfoTextString = FString("Players tied for the win : \n");
					for (const AEndlessBetrayalPlayerState* TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s \n"), *TiedPlayer->GetPlayerName()));
					}
				}
			}
			EndlessBetrayalHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoTextString));
		}
	}

	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(GetPawn());
	if(IsValid(EndlessBetrayalCharacter) && EndlessBetrayalCharacter->GetCombatComponent())
	{
		EndlessBetrayalCharacter->bShouldDisableGameplayInput = true;
		
		//Beware, the above line causes the character to stop being able to send info to the CombatComponent
		//If the player is already firing, it'll be stuck in firing, hence the following line
		EndlessBetrayalCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
}

void AEndlessBetrayalPlayerController::CheckPing(float DeltaSeconds)
{
	//Server has no lag so it doesn't make sense to check the ping
	if(HasAuthority()) return;
	
	HighPingRunningTime += DeltaSeconds;
	if(HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = !IsValid(PlayerState) ? GetPlayerState<APlayerState>() : PlayerState;
		if(IsValid(PlayerState))
		{
			//Note : If we were to use PlayerState->GetPing, the ping is compressed by UE to fit in a uint32.
			//The returned ping is 1/4 of the accurate one so we'd need to use GetPing * 4 to get the accurate one
			if(PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				ShowHighPingWarning();
				PingAnimationRunningTime = 0.0f;
			}
		}
		HighPingRunningTime = 0.0f;
	}

	const bool bIsHighPingAnimationPlaying = IsValid(EndlessBetrayalHUD) && IsValid(EndlessBetrayalHUD->CharacterOverlay) && IsValid(EndlessBetrayalHUD->CharacterOverlay->HighPingImage) && EndlessBetrayalHUD->CharacterOverlay->IsAnimationPlaying(EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation);
	if(bIsHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaSeconds;
		if(PingAnimationRunningTime > HighPingWarningDuration)
		{
			HideHighPingWarning();
		}
	}
}

void AEndlessBetrayalPlayerController::ShowHighPingWarning()
{
	EndlessBetrayalHUD = !IsValid(EndlessBetrayalHUD) ? EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD()) : EndlessBetrayalHUD;
	
	const bool bIsHUDValid = IsValid(EndlessBetrayalHUD) && IsValid(EndlessBetrayalHUD->CharacterOverlay) && IsValid(EndlessBetrayalHUD->CharacterOverlay->HighPingImage) && EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation;
	if(bIsHUDValid)
	{
		EndlessBetrayalHUD->CharacterOverlay->HighPingImage->SetOpacity(1.0f);
		EndlessBetrayalHUD->CharacterOverlay->PlayAnimation(EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation, 0.0f, 5);
	}
}

void AEndlessBetrayalPlayerController::HideHighPingWarning()
{
	EndlessBetrayalHUD = !IsValid(EndlessBetrayalHUD) ? EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD()) : EndlessBetrayalHUD;
	
	const bool bIsHUDValid = IsValid(EndlessBetrayalHUD) && IsValid(EndlessBetrayalHUD->CharacterOverlay) && IsValid(EndlessBetrayalHUD->CharacterOverlay->HighPingImage) && EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation;
	if(bIsHUDValid)
	{
		EndlessBetrayalHUD->CharacterOverlay->HighPingImage->SetOpacity(0.0f);
		if(EndlessBetrayalHUD->CharacterOverlay->IsAnimationPlaying(EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation))
		{
			EndlessBetrayalHUD->CharacterOverlay->StopAnimation(EndlessBetrayalHUD->CharacterOverlay->HighPingAnimation);
		}
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

void AEndlessBetrayalPlayerController::HandleMatchHasStarted()
{
	EndlessBetrayalHUD = !IsValid(EndlessBetrayalHUD) ? EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD()) : EndlessBetrayalHUD;
	if(IsValid(EndlessBetrayalHUD))
	{
		//HUD will only be displayed when the match is In Progress
		if(EndlessBetrayalHUD->CharacterOverlay == nullptr) EndlessBetrayalHUD->AddCharacterOverlay();
		if(IsValid(EndlessBetrayalHUD->AnnouncementWidget))
		{
			EndlessBetrayalHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AEndlessBetrayalPlayerController::ClientJoinMidGame_Implementation(const float InMatchTime, const float InWarmUpTime, const float InLevelStartingTime, const float InCooldownTime, const FName InMatchState)
{
	WarmUpTime = InWarmUpTime;
	MatchTime = InMatchTime;
	LevelStartingTime = InLevelStartingTime;
	MatchState = InMatchState;

	//Making sure that any update that needs to happen after the MatchState is set actually happens
	OnMatchStateSet(MatchState);

	EndlessBetrayalHUD = Cast<AEndlessBetrayalHUD>(GetHUD());
	if(IsValid(EndlessBetrayalHUD))
	{
		EndlessBetrayalHUD->AddAnnouncementWidget();
	}
}

void AEndlessBetrayalPlayerController::ServerCheckMatchState_Implementation()
{
	AEndlessBetrayalGameMode* GameMode = Cast<AEndlessBetrayalGameMode>(UGameplayStatics::GetGameMode(this));
	if(IsValid(GameMode))
	{
		WarmUpTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidGame(MatchTime, WarmUpTime, LevelStartingTime, CooldownTime, MatchState);
	}
}

void AEndlessBetrayalPlayerController::OnMatchStateSet(FName NewMatchState)
{
	MatchState = NewMatchState;
	
	HandleMatchStates();
}


void AEndlessBetrayalPlayerController::OnRep_MatchState()
{
	HandleMatchStates();
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
	float TimeLeft = 0.0f;
	//Reminder, GetServerTime() return the time since the game STARTED (including the time passed on menu)
	if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmUpTime  + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown) TimeLeft =  WarmUpTime  + MatchTime - GetServerTime() + LevelStartingTime + CooldownTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	//Has the BeginPlay of PC is called before the GameMode one, we ensure to correctly retrieve LevelStartingTime if on the server
	if(HasAuthority())
	{
		AEndlessBetrayalGameMode* GameMode = Cast<AEndlessBetrayalGameMode>(UGameplayStatics::GetGameMode(this));
		if(IsValid(GameMode))
		{
			EndlessBetrayalGameMode = GameMode;
			LevelStartingTime = GameMode->LevelStartingTime;

			SecondsLeft = FMath::CeilToInt(EndlessBetrayalGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if(CountDownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			UpdateHUDAnnouncementCountDown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			UpdateHUDMatchCountdown(TimeLeft);
		}
		
		UpdateHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountDownInt = SecondsLeft;
}

void AEndlessBetrayalPlayerController::HandleMatchStates()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
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
				UpdateShieldHUD(HUDShield, HUDMaxShield);
				UpdateScoreHUD(HUDScore);
				UpdateDeathsHUD(HUDDeaths);
				UpdateWeaponCarriedAmmo(HUDCarriedAmmo);
				UpdateWeaponAmmo(HUDWeaponAmmo);

				AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(GetCharacter());
				if(IsValid(EndlessBetrayalCharacter) && IsValid(EndlessBetrayalCharacter->GetCombatComponent()))
				{
					UpdateGrenadesAmmo(EndlessBetrayalCharacter->GetCombatComponent()->GetGrenadesAmount());
				}
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