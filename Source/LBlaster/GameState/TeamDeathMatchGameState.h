﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TeamDeathMatchGameState.generated.h"

/**
 * 
 */
UCLASS()
class LBLASTER_API ATeamDeathMatchGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddUniqueRedTeamPlayer(class ALBlasterPlayerState* InPlayerState);
	void AddUniqueBlueTeamPlayer(ALBlasterPlayerState* InPlayerState);
	void RemoveRedTeamPlayer(ALBlasterPlayerState* InPlayerState);
	void RemoveBlueTeamPlayer(ALBlasterPlayerState* InPlayerState);
	void SortRedTeamByKda();
	void SortBlueTeamByKda();
	
	FORCEINLINE int32 GetRedTeamNum() const { return RedTeam.Num(); }
	FORCEINLINE int32 GetBlueTeamNum() const { return BlueTeam.Num(); }
	const TArray<TObjectPtr<ALBlasterPlayerState>>& GetRedTeam() const { return RedTeam; }
	const TArray<TObjectPtr<ALBlasterPlayerState>>& GetBlueTeam() const { return BlueTeam; }

private:
	/*
	 *	Team
	 */
	UPROPERTY(ReplicatedUsing=OnRep_RedTeam)
	TArray<TObjectPtr<ALBlasterPlayerState>> RedTeam;

	UFUNCTION()
	void OnRep_RedTeam();

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing=OnRep_BlueTeam)
	TArray<TObjectPtr<ALBlasterPlayerState>> BlueTeam;

	UFUNCTION()
	void OnRep_BlueTeam();

	UPROPERTY(ReplicatedUsing=OnRep_BlueTeamScore)
	float BlueTeamScore;

	UFUNCTION()
	void OnRep_BlueTeamScore();
};
