// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//Our rocket is purely cosmetic
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if(IsValid(FiringPawn))
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
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
