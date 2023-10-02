// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Apply Damage here
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(IsValid(OwnerCharacter))
	{
		AEndlessBetrayalPlayerController* EventInstigator = Cast<AEndlessBetrayalPlayerController>(
			OwnerCharacter->GetController());
		if(IsValid(EventInstigator))
		{
			//Using UDamageType::StaticClass() as we don't have any damage type defined for now
			UGameplayStatics::ApplyDamage(OtherActor, GetDamage(), EventInstigator, this, UDamageType::StaticClass());
		}
	}
	
	//Super is called last because the parent version Destroys the Actor
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
