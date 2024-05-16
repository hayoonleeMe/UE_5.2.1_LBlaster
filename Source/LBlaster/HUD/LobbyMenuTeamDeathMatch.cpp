// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/LobbyMenuTeamDeathMatch.h"

#include "LobbyHUD.h"
#include "PlayerListRow.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "LBTypes/Team.h"
#include "Player/LBlasterPlayerState.h"
#include "Player/LobbyPlayerController.h"

void ULobbyMenuTeamDeathMatch::MenuSetup()
{
	Super::MenuSetup();

	if (RedTeamButton && !RedTeamButton->OnClicked.IsBound())
	{
		RedTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnRedTeamButtonClicked);
	}
	if (BlueTeamButton && !BlueTeamButton->OnClicked.IsBound())
	{
		BlueTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnBlueTeamButtonClicked);
	}
}

void ULobbyMenuTeamDeathMatch::AddNewPlayer(ALBlasterPlayerState* InPlayerState)
{
	if (RedTeamBox && BlueTeamBox && InPlayerState)
	{
		const int32 NumRedTeam = RedTeamBox->GetSlots().Num();
		const int32 NumBlueTeam = BlueTeamBox->GetSlots().Num();

		if (NumRedTeam <= NumBlueTeam)
		{
			AddRedTeam(InPlayerState, true);
		}
		else
		{
			AddBlueTeam(InPlayerState, true);
		}
	}
}

void ULobbyMenuTeamDeathMatch::AddRedTeam(ALBlasterPlayerState* InPlayerState, bool bDoBroadcast)
{
	if (RedTeamBox && IsValidOwnerController() && PlayerListRowClass && InPlayerState)
	{
		if (UPlayerListRow* PlayerListRow = CreateWidget<UPlayerListRow>(OwnerController, PlayerListRowClass))
		{
			InPlayerState->SetTeam(ETeam::ET_RedTeam);
			PlayerListRow->SetNameText(InPlayerState->GetPlayerName());
			RedTeamBox->AddChildToVerticalBox(PlayerListRow);
			if (bDoBroadcast)
			{
				BroadcastAddPlayerList(ETeam::ET_RedTeam, InPlayerState->GetPlayerName());
			}
		}
	}
}

void ULobbyMenuTeamDeathMatch::AddRedTeamForClient(const FString& InName)
{
	if (RedTeamBox && IsValidOwnerController() && PlayerListRowClass)
    {
    	if (UPlayerListRow* PlayerListRow = CreateWidget<UPlayerListRow>(OwnerController, PlayerListRowClass))
    	{
    		PlayerListRow->SetNameText(InName);
    		RedTeamBox->AddChildToVerticalBox(PlayerListRow);
    	}
    }
}

void ULobbyMenuTeamDeathMatch::AddBlueTeam(ALBlasterPlayerState* InPlayerState, bool bDoBroadcast)
{
	if (BlueTeamBox && IsValidOwnerController() && PlayerListRowClass && InPlayerState)
	{
		if (UPlayerListRow* PlayerListRow = CreateWidget<UPlayerListRow>(OwnerController, PlayerListRowClass))
		{
			InPlayerState->SetTeam(ETeam::ET_BlueTeam);
			PlayerListRow->SetNameText(InPlayerState->GetPlayerName());
			BlueTeamBox->AddChildToVerticalBox(PlayerListRow);
			if (bDoBroadcast)
			{
				BroadcastAddPlayerList(ETeam::ET_BlueTeam, InPlayerState->GetPlayerName());
			}
		}
	}
}

void ULobbyMenuTeamDeathMatch::AddBlueTeamForClient(const FString& InName)
{
	if (BlueTeamBox && IsValidOwnerController() && PlayerListRowClass)
	{
		if (UPlayerListRow* PlayerListRow = CreateWidget<UPlayerListRow>(OwnerController, PlayerListRowClass))
		{
			PlayerListRow->SetNameText(InName);
			BlueTeamBox->AddChildToVerticalBox(PlayerListRow);
		}
	}
}

void ULobbyMenuTeamDeathMatch::ChangeTeam(ETeam CurrentTeam, ETeam NewTeam, ALBlasterPlayerState* InPlayerState)
{
	// 기존 PlayerListRow 제거
	if (CurrentTeam == ETeam::ET_RedTeam)
	{
		RemovePlayerFromRedTeam(InPlayerState->GetPlayerName());
	}
	else if (CurrentTeam == ETeam::ET_BlueTeam)
	{
		RemovePlayerFromBlueTeam(InPlayerState->GetPlayerName());
	}

	// 새로운 팀에 추가
	if (IsValidOwnerPlayerState())
	{
		if (NewTeam == ETeam::ET_RedTeam)
		{
			AddRedTeam(InPlayerState, false);
		}
		else if (NewTeam == ETeam::ET_BlueTeam)
		{
			AddBlueTeam(InPlayerState, false);
		}	
	}

	BroadcastTeamChangePlayerList(CurrentTeam, NewTeam, InPlayerState->GetPlayerName());
}

void ULobbyMenuTeamDeathMatch::ChangeTeamForClient(ETeam CurrentTeam, ETeam NewTeam, const FString& InName)
{
	// 기존 PlayerListRow 제거
	if (CurrentTeam == ETeam::ET_RedTeam)
	{
		RemovePlayerFromRedTeam(InName);
	}
	else if (CurrentTeam == ETeam::ET_BlueTeam)
	{
		RemovePlayerFromBlueTeam(InName);
	}

	// 새로운 팀에 추가
	if (IsValidOwnerPlayerState())
	{
		if (NewTeam == ETeam::ET_RedTeam)
		{
			AddRedTeamForClient(InName);
		}
		else if (NewTeam == ETeam::ET_BlueTeam)
		{
			AddBlueTeamForClient(InName);
		}	
	}
}

void ULobbyMenuTeamDeathMatch::RemovePlayerFromRedTeam(const FString& InName)
{
	if (RedTeamBox)
	{
		for (int32 Index = RedTeamBox->GetSlots().Num() - 1; Index >= 0; --Index)
		{
			if (UPlayerListRow* Row = Cast<UPlayerListRow>(RedTeamBox->GetSlots()[Index]->Content))
			{
				// 해당 Row 삭제
				if (InName == Row->GetPlayerName())
				{
					RedTeamBox->RemoveChildAt(Index);
					break;
				}
			}
		}
	}
}

void ULobbyMenuTeamDeathMatch::RemovePlayerFromBlueTeam(const FString& InName)
{
	if (BlueTeamBox)
	{
		for (int32 Index = BlueTeamBox->GetSlots().Num() - 1; Index >= 0; --Index)
		{
			if (UPlayerListRow* Row = Cast<UPlayerListRow>(BlueTeamBox->GetSlots()[Index]->Content))
			{
				// 해당 Row 삭제
				if (InName == Row->GetPlayerName())
				{
					BlueTeamBox->RemoveChildAt(Index);
					break;
				}
			}
		}
	}
}

bool ULobbyMenuTeamDeathMatch::IsValidOwnerPlayerState()
{
	if (!OwnerPlayerState && IsValidOwnerController())
	{
		OwnerPlayerState = OwnerController->GetPlayerState<ALBlasterPlayerState>();
	}
	return OwnerPlayerState != nullptr;
}

void ULobbyMenuTeamDeathMatch::OnRedTeamButtonClicked()
{
	if (IsValidOwnerPlayerState())
	{
		if (OwnerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			return;
		}
		
		if (OwnerPlayerState->HasAuthority())
		{
			ChangeTeam(OwnerPlayerState->GetTeam(), ETeam::ET_RedTeam, OwnerPlayerState);	
		}
		else
		{
			if (IsValidOwnerController())
			{
				if (ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(OwnerController))
				{
					LobbyPlayerController->ServerSendTeamChangePlayerList(OwnerPlayerState->GetTeam(), ETeam::ET_RedTeam, OwnerPlayerState);
				}
			}			
		}
	}
}

void ULobbyMenuTeamDeathMatch::OnBlueTeamButtonClicked()
{
	if (IsValidOwnerPlayerState())
	{
		if (OwnerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			return;
		}
		
		if (OwnerPlayerState->HasAuthority())
		{
			ChangeTeam(OwnerPlayerState->GetTeam(), ETeam::ET_BlueTeam, OwnerPlayerState);
		}
		else
		{
			if (IsValidOwnerController())
			{
				if (ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(OwnerController))
				{
					LobbyPlayerController->ServerSendTeamChangePlayerList(OwnerPlayerState->GetTeam(), ETeam::ET_BlueTeam, OwnerPlayerState);
				}
			}	
		}
	}
}

void ULobbyMenuTeamDeathMatch::BroadcastAddPlayerList(ETeam InTeam, const FString& InName)
{
	// 서버의 PlayerList 추가를 연결된 모든 클라에 브로드캐스팅해 UI 동기화
	if (IsValidOwnerController() && OwnerController->HasAuthority() && IsValidOwnerHUD())
	{
		if (ALobbyHUD* LobbyHUD = Cast<ALobbyHUD>(OwnerHUD))
		{
			LobbyHUD->BroadcastAddPlayerList(InTeam, InName);
		}
	}
}

void ULobbyMenuTeamDeathMatch::BroadcastTeamChangePlayerList(ETeam CurrentTeam, ETeam NewTeam, const FString& InName)
{
	// 서버의 PlayerList 팀변경을 연결된 모든 클라에 브로드캐스팅해 UI 동기화
	if (IsValidOwnerController() && OwnerController->HasAuthority() && IsValidOwnerHUD())
	{
		if (ALobbyHUD* LobbyHUD = Cast<ALobbyHUD>(OwnerHUD))
		{
			LobbyHUD->BroadcastTeamChangePlayerList(CurrentTeam, NewTeam, InName);
		}
	}
}
