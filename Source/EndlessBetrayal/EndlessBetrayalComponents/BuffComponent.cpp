// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bIsHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bIsHealing || !IsValid(Character) || Character->IsEliminated()) return;

	const float HealPerFrame = HealingRate * DeltaTime;
	const float NewHealth = FMath::Clamp(Character->GetHealth() + HealPerFrame, 0.0f, Character->GetMaxHealth());
	Character->SetHealth(NewHealth);
	Character->UpdateHealthHUD();	//Cover the update of the Server HUD
	
	AmountToHeal -= HealPerFrame;
	if((AmountToHeal <= 0.0f) || (Character->GetHealth() >= Character->GetMaxHealth()) ) //Health should never be greater than MaxHealth, it's just a double security
	{
		bIsHealing = false;
		AmountToHeal -= 0.0f;
	}
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::Speed(float SpeedAmount, float CrouchSpeed, float SpeedingTime)
{
	if(!IsValid(Character)) return;

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, SpeedingTime);
	
	MulticastSpeedBuff(SpeedAmount, CrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchedSpeed)
{
	if(!IsValid(Character) || !Character->GetMovementComponent()) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedSpeed;
}

void UBuffComponent::ResetSpeeds()
{
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}
