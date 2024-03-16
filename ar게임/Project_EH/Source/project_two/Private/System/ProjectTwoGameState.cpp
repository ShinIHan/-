// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProjectTwoGameState.h"

void AProjectTwoGameState::AddPlayer(FString UserID, APlayerController* PlayerController)
{
	PlayerMap.Add(UserID, PlayerController);
}

void AProjectTwoGameState::RemovePlayer(FString UserID)
{
	if (PlayerMap.Find(UserID) != nullptr)
	{
		PlayerMap.Remove(UserID);
	}
}
