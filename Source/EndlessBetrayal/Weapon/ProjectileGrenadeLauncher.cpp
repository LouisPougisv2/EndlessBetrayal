// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenadeLauncher.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenadeLauncher::AProjectileGrenadeLauncher()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeLauncherMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//Following line makes sure that the projectile will keep its rotation aligned with the velocity
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenadeLauncher::BeginPlay()
{
	AActor::BeginPlay();

	ProjectileMovementComponent->OnProjectileBounce.AddUniqueDynamic(this, &AProjectileGrenadeLauncher::OnBounce);
	
	StartDestroyTimer();
	SpawnTrailSystem();
}

void AProjectileGrenadeLauncher::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BouncingCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BouncingCue, GetActorLocation());
	}
}
