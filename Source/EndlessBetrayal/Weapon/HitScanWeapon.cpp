// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(!IsValid(OwnerPawn)) return;
	AEndlessBetrayalPlayerController* DamageInstigator = Cast<AEndlessBetrayalPlayerController>(OwnerPawn->GetController());
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(IsValid(MuzzleFlashSocket))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if(IsValid(World))
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);

			FVector BeamEnd = End;
			
			if(FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				
				AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(FireHit.GetActor());
				//We only apply damage if we're on the Server
				if(IsValid(HitCharacter) && HasAuthority() && IsValid(DamageInstigator))
				{
					UGameplayStatics::ApplyDamage(HitCharacter, Damage, DamageInstigator, this, UDamageType::StaticClass());
				}

				if(IsValid(ImpactParticle))
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
				if(HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
				}
			}
			if(IsValid(BeamParticles))
			{
				BeamSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform);
				if(IsValid(BeamSystemComponent))
				{
					//Setting the end point for our Beam Particle system
					BeamSystemComponent->SetVectorParameter(TEXT("Target"), BeamEnd);
				}
			}
		}

		if(IsValid(MuzzleFlash))
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
		}
		
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

FVector AHitScanWeapon::GetTraceEndWithScatter(const FVector& TraceStartLocation, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStartLocation).GetSafeNormal();
	FVector SphereCenter = TraceStartLocation + ToTargetNormalized * DistanceToSphere;
	FVector	RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SphereRadius);
	FVector EndLocation = SphereCenter + RandVect;
	FVector ToEndLocation = EndLocation - TraceStartLocation;
	
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Yellow, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.0f, 12, FColor::Red, true);
	DrawDebugLine(GetWorld(), TraceStartLocation, FVector(TraceStartLocation + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::White, true);
	
	return FVector(TraceStartLocation + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}
