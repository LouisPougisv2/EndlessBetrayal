// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/BuffComponent.h"

AShieldPickup::AShieldPickup()
{
	bReplicates = true;
}

void AShieldPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if(IsValid(EndlessBetrayalCharacter) && IsValid(EndlessBetrayalCharacter->GetBuffComponent()))
	{
		EndlessBetrayalCharacter->GetBuffComponent()->RestoreShield(ShieldAmount, RestoringShieldTime);
	}
	Destroy();
}
