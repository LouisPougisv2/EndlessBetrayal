// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if(IsValid(EndlessBetrayalCharacter) && IsValid(EndlessBetrayalCharacter->GetCombatComponent()))
	{
		EndlessBetrayalCharacter->GetCombatComponent()->PickupAmmo(AmmoWeaponType, AmmoAmount);
	}
	Destroy();
}
