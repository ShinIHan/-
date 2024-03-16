// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ProjectTwoGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_TWO_API AProjectTwoGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, APlayerController*> PlayerMap;

public:
	void AddPlayer(FString UserID, APlayerController* PlayerController);
	void RemovePlayer(FString UserID);
};
