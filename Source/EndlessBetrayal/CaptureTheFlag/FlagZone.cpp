// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "EndlessBetrayal/GameMode/CaptureTheFlagGameMode.h"
#include "EndlessBetrayal/Weapon/Flag.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	FlagZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FlagZoneSphere"));
	SetRootComponent(FlagZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	FlagZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlapBegin);
}

void AFlagZone::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlagActor = Cast<AFlag>(OtherActor);
	if(IsValid(OverlappingFlagActor) && OverlappingFlagActor->GetTeam() != GetFlagZoneTeam())
	{
		//Score point
		ACaptureTheFlagGameMode* CaptureTheFlagGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if(IsValid(CaptureTheFlagGameMode))
		{
			CaptureTheFlagGameMode->FlagCaptured(OverlappingFlagActor, this);
		}
		OverlappingFlagActor->ResetFlag();
	}
}

