// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	SetRootComponent(GetWeaponMesh());

	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

	GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
