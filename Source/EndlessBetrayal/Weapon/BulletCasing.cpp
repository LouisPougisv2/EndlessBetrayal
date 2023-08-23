// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletCasing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletCasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletCasingMesh"));
	BulletCasingMesh->SetupAttachment(RootComponent);

	BulletCasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	BulletCasingMesh->SetSimulatePhysics(true);
	BulletCasingMesh->SetEnableGravity(true);
	BulletCasingMesh->SetNotifyRigidBodyCollision(true);
	BulletCasingEjectImpulse = 10.0f;
	SetLifeSpan(2.0f);
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();

	BulletCasingMesh->OnComponentHit.AddUniqueDynamic(this, &ABulletCasing::OnHit);
	BulletCasingMesh->AddImpulse(GetActorForwardVector() * BulletCasingEjectImpulse);
}

void ABulletCasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(ImpactSound)
	{
		//Play the sound at the bullet casing location when it hits the ground
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	BulletCasingMesh->SetNotifyRigidBodyCollision(false);
}

