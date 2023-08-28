// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//Spawning the projectile will only happen if the weapon exists on the server
	if(!HasAuthority()) return;
	
	if(IsValid(GetWeaponMesh()))
	{
		const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
		if(IsValid(MuzzleFlashSocket))
		{
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

			//Rotation of "From Muzzle Flash socket to Hit Target (from TraceUnderCrosshair)"
			FRotator TargetRotation = (HitTarget - SocketTransform.GetLocation()).Rotation();
			
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = CastChecked<APawn>(GetOwner());
			
			if(!ensureAlways(IsValid(ProjectileClass))) return;

			UWorld* World = GetWorld();
			if(IsValid(World))
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
			}
		}
	}
}
