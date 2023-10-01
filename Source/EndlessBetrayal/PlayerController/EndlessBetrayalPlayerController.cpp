// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/HUD/CharacterOverlay.h"
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"

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
	}
}

void AEndlessBetrayalPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	const AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(InPawn);
	UpdateHealthHUD(EndlessBetrayalCharacter->GetHealth(), EndlessBetrayalCharacter->GetMaxHealth());

}

void AEndlessBetrayalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EndlessBetrayalHUD= Cast<AEndlessBetrayalHUD>(GetHUD());
}
