// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalAnimInstance.h"
#include "EndlessBetrayalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEndlessBetrayalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(TryGetPawnOwner());
}

void UEndlessBetrayalAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (EndlessBetrayalCharacter == nullptr)
	{
		EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(TryGetPawnOwner());
	}
	if (EndlessBetrayalCharacter == nullptr) return;

	FVector Velocity = EndlessBetrayalCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	bIsInAir = EndlessBetrayalCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = EndlessBetrayalCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
}
