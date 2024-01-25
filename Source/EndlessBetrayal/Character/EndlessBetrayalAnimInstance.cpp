// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalAnimInstance.h"
#include "EndlessBetrayalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EndlessBetrayal/Weapon/Weapon.h"


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

	bWeaponEquipped = EndlessBetrayalCharacter->IsWeaponEquipped();
	EquippedWeapon = EndlessBetrayalCharacter->GetEquippedWeapon();
	bIsCrouched = EndlessBetrayalCharacter->bIsCrouched;		//Coming from the character.h boolean
	bIsAiming = EndlessBetrayalCharacter->IsAiming();
	TurningInPlace = EndlessBetrayalCharacter->GetTurningInPlace();
	bShouldRotateRootBone = EndlessBetrayalCharacter->ShouldRotateRootBone();
	bIsEliminated = EndlessBetrayalCharacter->IsEliminated();

	//Offset yawfor straffing
	FRotator AimRotation = EndlessBetrayalCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(EndlessBetrayalCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.0f);	//Will avoid to interp from -180->0>180, instead, will take the shortest path (-180->180)
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = EndlessBetrayalCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;	//This is scaling up and make it proportional to DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.0); //To avoid jerkiness in our lean
	Lean = FMath::Clamp(Interp, -90.0, 90.0);	//To avoid to go below/above -90.0 or 90.0 when we move the mouse quickly

	AO_Yaw = EndlessBetrayalCharacter->GetAO_Yaw();
	AO_Pitch = EndlessBetrayalCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && EndlessBetrayalCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;	//Position of the left hand socket on the weapon transformed to hand_r bone space
		FRotator OutRotation;	//Rotation of the left hand socket on the weapon transformed to hand_r bone space

		//We now want the transform world space to be converted in bone space
		EndlessBetrayalCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if(EndlessBetrayalCharacter->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = EndlessBetrayalCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - EndlessBetrayalCharacter->GetHitTarget()));

			//TODO : To keep for future tests
			//const FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(("MuzzleFlash"), RTS_World);
			//const FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
			//
			//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.0f, FColor::Red);
			//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), EndlessBetrayalCharacter->GetHitTarget(), FColor::Green);
		}

	}
	bShouldUseFabrik = EndlessBetrayalCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && EndlessBetrayalCharacter->GetCombatState() != ECombatState::ECS_SwappingWeapons;
}
