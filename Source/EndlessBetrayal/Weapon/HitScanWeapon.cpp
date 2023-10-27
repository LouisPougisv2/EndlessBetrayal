// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(!IsValid(OwnerPawn)) return;
	AEndlessBetrayalPlayerController* DamageInstigator = Cast<AEndlessBetrayalPlayerController>(OwnerPawn->GetController());
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(IsValid(MuzzleFlashSocket) && IsValid(DamageInstigator))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if(IsValid(World))
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			if(FireHit.bBlockingHit)
			{
				AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(FireHit.GetActor());
				//We only apply damage if we're on the Server
				if(IsValid(HitCharacter) && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(HitCharacter, Damage, DamageInstigator, this, UDamageType::StaticClass());
				}

				if(ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
			}
		}
	}
}
