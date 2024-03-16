// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "../Plugins//ServerManagerConnector/Source/ServerManagerConnector/Public/NetworkTCPSubsystem.h"

#include "ProjectTwoGameInstance.generated.h"

UENUM(BlueprintType)
enum class ENetworkType : uint8
{
	TE_None UMETA(DisplayName = "None"),
	TE_Local UMETA(DisplayName = "Local"),
	TE_Cloud UMETA(DisplayName = "Cloud"),
};

/**
 * 
 */
UCLASS()
class PROJECT_TWO_API UProjectTwoGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UProjectTwoGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ENetworkType Type;
};
