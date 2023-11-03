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
		FHitResult FireHit;

		WeaponTraceHit(Start, HitTarget, FireHit);
			
		AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(FireHit.GetActor());
		//We only apply damage if we're on the Server
		if(IsValid(HitCharacter) && HasAuthority() && IsValid(DamageInstigator))
		{
			UGameplayStatics::ApplyDamage(HitCharacter, Damage, DamageInstigator, this, UDamageType::StaticClass());
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

	if(IsValid(World))
	{
		const FVector End = bUseScatter ? GetTraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(OutHitResult, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}

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

FVector AHitScanWeapon::GetTraceEndWithScatter(const FVector& TraceStartLocation, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStartLocation).GetSafeNormal();
	FVector SphereCenter = TraceStartLocation + ToTargetNormalized * DistanceToSphere;
	FVector	RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SphereRadius);
	FVector EndLocation = SphereCenter + RandVect;
	FVector ToEndLocation = EndLocation - TraceStartLocation;
	
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Yellow, true);
	//DrawDebugSphere(GetWorld(), EndLocation, 4.0f, 12, FColor::Red, true);
	//DrawDebugLine(GetWorld(), TraceStartLocation, FVector(TraceStartLocation + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::White, true);
	
	return FVector(TraceStartLocation + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}
