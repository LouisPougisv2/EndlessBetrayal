// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	if(IsValid(GetWeaponMesh()))
	{
		APawn* InstigatorPawn = CastChecked<APawn>(GetOwner());
		const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
		UWorld* World = GetWorld();
		
		if(IsValid(MuzzleFlashSocket) && IsValid(World))
		{
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

			//Rotation of "From Muzzle Flash socket to Hit Target (from TraceUnderCrosshair)"
			FRotator TargetRotation = (HitTarget - SocketTransform.GetLocation()).Rotation();
			
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;

			if(!ensureAlways(IsValid(ProjectileClass)) || !ensureAlways(IsValid(ServerSideRewindProjectileClass))) return;
			
			AProjectile* SpawnedProjectile = nullptr;
			if(bUseServerSideRewind)
			{
				if(InstigatorPawn->HasAuthority()) //Server
				{
					if(InstigatorPawn->IsLocallyControlled())	//Server, Host - use replicated projectile
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false; //Already false but for clarification
						SpawnedProjectile->SetDamage(GetDamage());
					}
					else //Server, NOT locally controlled - spawn non-replicated projectile, Server Side Rewind
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
					}
				}
				else //Client, using ServerSide Rewind
				{
					if(InstigatorPawn->IsLocallyControlled()) //Client, Locally Controlled - Spawn non replicated projectile, use ServerSide Rewind
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed; //Scaled by initial speed as Forward vector is normalized
						SpawnedProjectile->SetDamage(GetDamage());
					}
					else //Client, NOT Locally Controlled - Spawn non replicated projectile, no Server Side Rewind
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
						SpawnedProjectile->bUseServerSideRewind = false;
					}
				}
			}
			else //Weapon not using Server Side Rewind
			{
				if(InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUseServerSideRewind = false; //Already false but for clarification
					SpawnedProjectile->SetDamage(GetDamage());
				}
			}
		}
	}
}
