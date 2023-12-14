// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "EndlessBetrayal/Weapon/WeaponTypes.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(75.0f);
	CollisionSphere->AddLocalOffset(FVector(0.0f, 0.0f, 60.0f));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	PickupMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	PickupMesh->SetupAttachment(CollisionSphere);	//So if we move the collision sphere, the mesh follows
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Mesh Outline Effect
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	//As it is a replicated actor, we only want the overlap on the server
	if(HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapDelegate, BindOverlapTime);	
	}
}

void APickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickup::BindOverlapDelegate()
{
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlapBegin);
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(IsValid(PickupMesh) && ShouldPickupRotate)
	{
		PickupMesh->AddLocalRotation(FRotator(0.0f, BaseRotatingRate * DeltaTime, 0.0f));
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation(), GetActorRotation());
	}

	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
	}
}

