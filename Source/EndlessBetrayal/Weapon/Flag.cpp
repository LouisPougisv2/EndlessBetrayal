// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"

AFlag::AFlag()
{
	//bReplicates = true;
	//SetReplicateMovement(true);

	SetRootComponent(GetWeaponMesh());

	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

	GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetTransform();
}

void AFlag::ResetFlag()
{
	AEndlessBetrayalCharacter* FlagBearer = Cast<AEndlessBetrayalCharacter>(GetOwner());
	if(IsValid(FlagBearer))
	{
		FlagBearer->SetHoldingFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	if (!HasAuthority()) return;
	
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	GetWeaponMesh()->DetachFromComponent(DetachmentTransformRules);
	SetWeaponState(EWeaponState::EWS_Initial);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetOwner(nullptr);
	WeaponOwnerCharacter = nullptr;
	WeaponOwnerController = nullptr;
	
	SetActorTransform(InitialTransform);
	MulticastResetFlagTransform();
}

void AFlag::MulticastResetFlagTransform_Implementation()
{
	SetActorTransform(InitialTransform);
}

void AFlag::HandleWeaponEquipped()
{
	Super::HandleWeaponEquipped();

	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetWeaponMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}
