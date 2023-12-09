// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENDLESSBETRAYAL_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UBuffComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	friend class AEndlessBetrayalCharacter;	//As Combat component will need to access a lot of variable from The character

	void Heal(float HealAmount, float HealingTime);

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void Speed(float BaseSpeed, float CrouchSpeed, float SpeedingTime);

protected:
	
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

private:	

	UPROPERTY()
	class AEndlessBetrayalCharacter* Character;		//Set up in the PostInitializeComponent function in the EndlessBetrayalCharacter


	/*
	* Healing Buff
	*/

	bool bIsHealing = false;
	float HealingRate = 0.0f;
	float AmountToHeal = 0.0f;

	/*
	* Speeding Buff
	*/

	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchedSpeed);
	void ResetSpeeds();
		
};