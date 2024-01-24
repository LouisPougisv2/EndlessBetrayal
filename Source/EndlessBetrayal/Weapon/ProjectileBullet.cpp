// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/LagCompensationComponent.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//Following line makes sure that the projectile will keep its rotation aligned with the velocity
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if(IsValid(ProjectileMovementComponent))
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*FPredictProjectilePathParams PathParams;
	//PredictProjectilePathParams.bTraceComplex -> if true, traces against the mesh itself instead of collisions or assets
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;	//Allows us to predict a projectile path and generate hit events as we're tracing against the collision object in our world
	PathParams.DrawDebugTime = 5.0f;	//TODO : To remove
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;	//TODO : To remove
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.0f; //Amount of time the projectile would be flying through the air
	PathParams.ProjectileRadius = 5.0f;
	PathParams.SimFrequency = 20.0f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);
	
	FPredictProjectilePathResult PredictProjectilePathResult;
	
	UGameplayStatics::PredictProjectilePath(this, PathParams, PredictProjectilePathResult);*/
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Apply Damage here
	AEndlessBetrayalCharacter* OwnerCharacter = Cast<AEndlessBetrayalCharacter>(GetOwner());
	if(IsValid(OwnerCharacter))
	{
		AEndlessBetrayalPlayerController* EventInstigator = Cast<AEndlessBetrayalPlayerController>(OwnerCharacter->GetController());
		if(IsValid(EventInstigator))
		{
			if(OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				//Using UDamageType::StaticClass() as we don't have any damage type defined for now
				UGameplayStatics::ApplyDamage(OtherActor, GetDamage(), EventInstigator, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			AEndlessBetrayalCharacter* HitCharacter = Cast<AEndlessBetrayalCharacter>(OtherActor);
			if(IsValid(HitCharacter) && OwnerCharacter->IsLocallyControlled() && bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent())
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, TraceStart, InitialVelocity, EventInstigator->GetServerTime() - EventInstigator->SingleTripTime);
			}			
		}
	}
	
	//Super is called last because the parent version Destroys the Actor
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
