// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LBlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LBLASTER_API ALBlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALBlasterPlayerController();
	virtual void Tick(float DeltaSeconds) override;
	// Synced with Server world clock
	virtual float GetServerTime();
	// Sync with server clock as soon as possible (Called after this PlayerController's Viewport/net connection is associated with this player controller)
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float InScore);
	void SetHUDDeath(int32 InDeath);
	void SetHUDAmmo(int32 InAmmo);
	void SetHUDCarriedAmmo(int32 InCarriedAmmo);
	void SetHUDWeaponTypeText(const FString& InWeaponTypeString = FString());
	void SetHUDMatchCountdown(float InCountdownTime);
	void OnMatchStateSet(FName InState);
	void UpdateHUDHealth();
	void SetHUDAnnouncementCountdown(float InCountdownTime);
	void UpdateHUDGrenadeAmount();
	void UpdateHUDGrenadeAmount(int32 InGrenadeAmount);
	
	void HandleMatchHasStarted();
	void HandleCooldown();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDTime();

	/*
	 *	Sync time between server and client
	 */
	// current server time을 요청. 요청이 전달될 때의 client time을 전달.
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// ServerRequestServerTime에 대한 응답으로 클라이언트로 current Server Time 전달.
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category="LBlaster|Time")
	float TimeSyncFrequency;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);
	
private:
	UPROPERTY()
	TObjectPtr<class ALBlasterHUD> LBlasterHUD;

	bool IsValidHUD();

	UPROPERTY()
	TObjectPtr<class ALBlasterGameMode> LBlasterGameMode;

	bool IsValidGameMode();

	/*
     *	Match Countdown
     */
	float LevelStartingTime = 0.f;
    float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
    uint8 CountdownInt = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
};
