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

	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateHealthHUD(float NewHealth, float MaxHealth);
	void UpdateScoreHUD(float NewScore);
	void UpdateDeathsHUD(int32 NewDeath);
	void UpdateHUDMatchCountdown(float CountdownTime);
	void UpdateHUDAnnouncementCountDown(float CountdownTime);
	void HideMessagesOnScreenHUD();
	void UpdateWeaponAmmo(int32 NewAmmo);
	void UpdateWeaponCarriedAmmo(int32 NewAmmo);
	
	//Sync with Server clock as soon as possible
	virtual void ReceivedPlayer() override;

	//Only happening on the server
	void OnMatchStateSet(FName NewMatchState);
	
	//synced with Server world clock 
	virtual float GetServerTime();
	void PollInit();
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void CheckTimeSync(float DeltaSeconds);
	void SetHUDTime();
	void HandleMatchHasStarted();

	/**
	* Sync Time between Client and Server
	*/

	//Request the current server time passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(const float InMatchTime, const float InWarmUpTime, const float InLevelStartingTime, const FName InMatchState);
	
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
	float MatchTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float WarmUpTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float LevelStartingTime = 0.0f;

	UPROPERTY(EditAnywhere)
	uint32 CountDownInt;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bShouldInitializeCharacterOverlay = false;

	//TODO : Remove during refactor
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDDeaths;
};
