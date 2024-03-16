// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ProjectTwoPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_TWO_API AProjectTwoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString PlayerSub;	// 
	
};
