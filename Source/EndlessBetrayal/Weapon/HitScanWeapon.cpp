// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/CombatComponent.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(!IsValid(OwnerPawn)) return;
	AController* DamageInstigator = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(IsValid(MuzzleFlashSocket))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;

		WeaponTraceHit(Start, HitTarget, FireHit);
			
		AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(FireHit.GetActor());
		//We only apply damage if we're on the Server
		if(IsValid(HitCharacter) && IsValid(DamageInstigator))
		{
			//If next bool is true, the server should cause damage
			bool bCauseAuthDamage =	!bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if(HasAuthority() && bCauseAuthDamage)
			{
				//We can retrieve the head bone from the physic asset in order to check if it is a headshot
				const float DamageToApply = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(HitCharacter, DamageToApply, DamageInstigator, this, UDamageType::StaticClass());
			}
			
			if(!HasAuthority() &&bUseServerSideRewind)
			{
				WeaponOwnerCharacter = !IsValid(WeaponOwnerCharacter) ? Cast<AEndlessBetrayalCharacter>(OwnerPawn) : WeaponOwnerCharacter;
				WeaponOwnerController = !IsValid(WeaponOwnerController) ? Cast<AEndlessBetrayalPlayerController>(DamageInstigator) : WeaponOwnerController;

				//We only want to send the Client RPC if the client is locally controlled
				if(IsValid(WeaponOwnerCharacter) && IsValid(WeaponOwnerController) && WeaponOwnerCharacter->GetLagCompensationComponent() && WeaponOwnerCharacter->IsLocallyControlled())
				{
					//Make sure the server rewind time enough to position that opponent's character to the location that it was in corresponding to when we hit
					float HitTime = WeaponOwnerController->GetServerTime() - WeaponOwnerController->SingleTripTime;
					WeaponOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(HitCharacter, Start, FireHit.ImpactPoint, HitTime);
				}
			}
		}

		if(IsValid(ImpactParticle))
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
		
		if(HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		
		if(IsValid(MuzzleFlash))
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult)
{
	UWorld* World = GetWorld();

	if(GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		AEndlessBetrayalCharacter* Character = Cast<AEndlessBetrayalCharacter>(GetWeaponOwnerCharacter());
		if(IsValid(Character) && Character->GetCombatComponent())
		{
			bUseScatter = !Character->GetCombatComponent()->IsAiming();
		}
	}
	
	if(IsValid(World))
	{
		const FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(OutHitResult, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}
		else
		{
			OutHitResult.ImpactPoint = End;
		}
		
		//DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		
		if(IsValid(BeamParticles))
		{
			BeamSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart);
			if(IsValid(BeamSystemComponent))
			{
				//Setting the end point for our Beam Particle system
				BeamSystemComponent->SetVectorParameter(TEXT("Target"), BeamEnd);
			}
		}
	}
}