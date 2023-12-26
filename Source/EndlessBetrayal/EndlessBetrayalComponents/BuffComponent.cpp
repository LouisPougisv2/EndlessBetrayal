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
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::BuffHeal(float HealAmount, float HealingTime)
{
	bIsHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::RestoreShield(float ShieldAmount, float RestoringShieldTime)
{
	bIsRestoringShield = true;
	RestoringShieldRate = ShieldAmount / RestoringShieldTime;
	AmountToShield += ShieldAmount;
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

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bIsRestoringShield || !IsValid(Character) || Character->IsEliminated()) return;

	const float RestoringShieldPerFrame = RestoringShieldRate * DeltaTime;
	const float NewShield = FMath::Clamp(Character->GetShield() + RestoringShieldPerFrame, 0.0f, Character->GetMaxShield());
	Character->SetShield(NewShield);
	Character->UpdateShieldHUD();	//Cover the update of the Server HUD
	
	AmountToShield -= RestoringShieldPerFrame;
	if((AmountToShield <= 0.0f) || (Character->GetShield() >= Character->GetMaxShield()) ) //Shield should never be greater than MaxShield, it's just a double security
	{
		bIsRestoringShield = false;
		AmountToShield = 0.0f;
	}
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float SpeedAmount, float CrouchSpeed, float SpeedingTime)
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

void UBuffComponent::SetInitialJumpVelocity(float JumpVelocity)
{
	InitialJumpVelocity = JumpVelocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if(!IsValid(Character)) return;

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJumpVelocity, BuffTime);
	
	MulticastJumpBuff(BuffJumpVelocity);
}


void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(!IsValid(Character) || !Character->GetMovementComponent()) return;

	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;

}

void UBuffComponent::ResetJumpVelocity()
{
	MulticastJumpBuff(InitialJumpVelocity);
}
