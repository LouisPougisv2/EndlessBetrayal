// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/BuffComponent.h"

AJumpPickup::AJumpPickup()
{
	bReplicates = true;
}

void AJumpPickup::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AEndlessBetrayalCharacter* EndlessBetrayalCharacter = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if(IsValid(EndlessBetrayalCharacter) && IsValid( EndlessBetrayalCharacter->GetBuffComponent()))
	{
		EndlessBetrayalCharacter->GetBuffComponent()->BuffJump(JumpZVelocityBuff, JumpBuffTime);
	}
	Destroy();
}
