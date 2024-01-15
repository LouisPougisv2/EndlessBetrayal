// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//Contains the barebones information we need for a given box
USTRUCT()
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
	
};


//Struct that stores the information about all the HitBoxes for the character, for a given frame
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInformationMap;

	UPROPERTY()
	class AEndlessBetrayalCharacter* Character = nullptr;
	
};

USTRUCT(BlueprintType)
struct FServerSideRewindResults
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	bool bIsAHeadshot = false;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResults
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AEndlessBetrayalCharacter*, uint32> HeadShotsMap;

	UPROPERTY()
	TMap<AEndlessBetrayalCharacter*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENDLESSBETRAYAL_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	friend class AEndlessBetrayalCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FServerSideRewindResults ServerSideRewind(AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);

	/**
	* Shotgun 
	*/

	FShotgunServerSideRewindResults ShotgunServerSideRewind(const TArray<AEndlessBetrayalCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AEndlessBetrayalCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	
protected:

	virtual void BeginPlay() override;

	void TickSaveFramePackage();
	void SaveFramePackage(FFramePackage& InFramePackage);

	void ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Color);

	FFramePackage GetFrameToCheck(AEndlessBetrayalCharacter* HitCharacter, float HitTime);
	
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime);

	FServerSideRewindResults ConfirmHit(const FFramePackage& FrameToCheck, AEndlessBetrayalCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	FShotgunServerSideRewindResults ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

	void CheckShotgunShots(TMap<AEndlessBetrayalCharacter*, uint32>& MapShots, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);
	
	void ToggleCharacterMeshCollision(AEndlessBetrayalCharacter* HitCharacter, ECollisionEnabled::Type NewType);

	void CacheBoxPositions(AEndlessBetrayalCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveBoxes(AEndlessBetrayalCharacter* HitCharacter, const FFramePackage& FrameToMoveAt);
	
	void ResetHitBoxes(AEndlessBetrayalCharacter* HitCharacter, const FFramePackage& FrameToMoveAt);

	
private:

	UPROPERTY()
	AEndlessBetrayalCharacter* Character;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* PlayerController;

	//Frame History for the last 400ms 
	//Note : Not designed for exposure to BP so cannot have UPROPERTY()
	TDoubleLinkedList<FFramePackage> FramesHistory;

	//Max time after which a FramePackage is discarded from the FrameHistory
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 0.4f;
};
