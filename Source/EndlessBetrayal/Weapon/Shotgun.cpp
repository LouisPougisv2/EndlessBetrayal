// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
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
