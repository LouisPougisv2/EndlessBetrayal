// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Our rocket is purely cosmetic

	InitialSpeed = 2000.0f;
	
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	//Following line makes sure that the projectile will keep its rotation aligned with the velocity
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
	RocketMovementComponent->InitialSpeed = InitialSpeed;
	RocketMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileRocket::Destroyed()
{
	
}
//To keep for development
//void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
//	{
//		RocketMovementComponent->InitialSpeed = InitialSpeed;
//		RocketMovementComponent->MaxSpeed = InitialSpeed;
//	}
//}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddUniqueDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if(IsValid(RocketSoundCue) && IsValid(RocketSoundAttenuation))
	{
		RocketSoundComponent = UGameplayStatics::SpawnSoundAttached(RocketSoundCue,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.0f,
			1.0f,
			0.0f,
			RocketSoundAttenuation,
			nullptr,
			false);
		
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ExplodeDamage();
	
	StartDestroyTimer();
	UpdateRocketEffects();
}

void AProjectileRocket::UpdateRocketEffects()
{
	if(IsValid(ImpactParticles))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if(SmokeTrailComponent)
	{
		SmokeTrailComponent->GetSystemInstanceController()->Deactivate();
	}
	
	if(RocketSoundComponent && RocketSoundComponent->IsPlaying())
	{
		RocketSoundComponent->Stop();
	}
}


