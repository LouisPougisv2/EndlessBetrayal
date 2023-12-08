// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/BuffComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void AHealthPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if(IsValid(EndlessBetrayalCharacter) && IsValid(EndlessBetrayalCharacter->GetBuffComponent()))
	{
		EndlessBetrayalCharacter->GetBuffComponent()->Heal(HealthAmount, HealingTime);
	}
	Destroy();
}

void AHealthPickup::Destroyed()
{
	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
	}

	Super::Destroyed();
}
