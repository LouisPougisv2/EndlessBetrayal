// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystemInstanceController.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Our rocket is purely cosmetic

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	//Following line makes sure that the projectile will keep its rotation aligned with the velocity
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::Destroyed()
{
	
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddUniqueDynamic(this, &AProjectileRocket::OnHit);
	}

	if(SmokeTrailSystem)
	{
		SmokeTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(SmokeTrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

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
	if(OtherActor == GetOwner())
	{
		return;
	}
	APawn* FiringPawn = GetInstigator();
	if(IsValid(FiringPawn) && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if(IsValid(FiringController))
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,	//World Context
				Damage,	//Base Damage
				MinimumDamage,	//Minimum inflicted damage
				GetActorLocation(),	//Origin
				DamageInnerRadius,	//Damage Inner Radius
				DamageOuterRadius,	//Damage Outer Radius
				1.0f,	//Damage Fall off
				UDamageType::StaticClass(),	//Damage Type
				TArray<AActor*>(),	//Ignored actors
				this,	//Damage Causer
				FiringController //Instigator Controller
				);	
		}
	}
	//Super call is destroying the actor (with Particles and sound), perform any functionality before
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyOnTimerFinished, DestroyTime);

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

	if(RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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

void AProjectileRocket::DestroyOnTimerFinished()
{
	Destroy();
}


