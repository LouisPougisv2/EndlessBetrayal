// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EndlessBetrayalPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void UpdateHealthHUD(float NewHealth, float MaxHealth);
	void UpdateScoreHUD(float NewScore);
	void UpdateDeathsHUD(int32 NewDeath);
	void UpdateHUDMatchCountdown(float CountdownTime);
	void HideMessagesOnScreenHUD();

	/**
	 * Ammo
	 */
	void UpdateWeaponAmmo(int32 NewAmmo);
	void UpdateWeaponCarriedAmmo(int32 NewAmmo);
	
	virtual void OnPossess(APawn* InPawn) override;
	//Sync with Server clock as soon as possible
	virtual void ReceivedPlayer() override;
	
	//synced with Server world clock 
	virtual float GetServerTime();
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void CheckTimeSync(float DeltaSeconds);
	void SetHUDTime();

	/**
	* Sync Time between Client and Server
	*/

	//Request the current server time passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	
	//Difference between Client and Server Time
	UPROPERTY()
	float ClientServerDelta = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.0f;

	UPROPERTY()
	float TimeSyncRunningTime = 0.0f;
	
private:

	UPROPERTY()
	class AEndlessBetrayalHUD* EndlessBetrayalHUD;

	//Will be moved to GameMode when Match States will come
	UPROPERTY(EditAnywhere)
	float MatchTime = 180.0f;

	UPROPERTY(EditAnywhere)
	uint32 CountDownInt;
	
};
