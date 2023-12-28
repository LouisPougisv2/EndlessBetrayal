// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(!IsValid(OwnerPawn)) return;
	AEndlessBetrayalPlayerController* DamageInstigator = Cast<AEndlessBetrayalPlayerController>(OwnerPawn->GetController());
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(IsValid(MuzzleFlashSocket))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<AEndlessBetrayalCharacter*, uint32> PlayersHitMap;
		for(uint32 i = 0; i < NumberOfPellets; ++i)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(FireHit.GetActor());

			if(IsValid(HitCharacter) && HasAuthority() && IsValid(DamageInstigator))
			{
				if(PlayersHitMap.Contains(HitCharacter))
				{
					++PlayersHitMap[HitCharacter];
				}
				else
				{
					PlayersHitMap.Emplace(HitCharacter, 1);
				}
			}

			if(IsValid(ImpactParticle))
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
		
			if(HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
			}
		}

		for (TPair<AEndlessBetrayalCharacter*, unsigned> PlayerHit : PlayersHitMap)
		{
			if(IsValid(PlayerHit.Key) && HasAuthority() && IsValid(DamageInstigator))
			{
				UGameplayStatics::ApplyDamage(PlayerHit.Key, Damage * PlayerHit.Value, DamageInstigator, this, UDamageType::StaticClass());
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(!IsValid(MuzzleFlashSocket)) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	
	
	for (int i = 0; i < NumberOfPellets; ++i)
	{
		const FVector	RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandVect;
		FVector ToEndLocation = EndLocation - Start;
		ToEndLocation = (Start + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
		
		HitTargets.Add(ToEndLocation);
	}
}
