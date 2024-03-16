// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Network/ResourceUsageSender.h"
#include "System/ProjectTwoGameInstance.h"
#include "System/ProjectTwoPlayerState.h"
#include "System/ProjectTwoGameState.h"

#include "ProjectTwoGameModeBase.generated.h"

USTRUCT(BlueprintType)
struct FTimeTableLectureInfo
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(BlueprintReadWrite)
	FString Classroom;

	UPROPERTY(BlueprintReadWrite)
	FString Content;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> StudentSubs;
};

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECT_TWO_API AProjectTwoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProjectTwoGameModeBase();
	
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;

	void Start();
	void Stop();
	
protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	UResourceUsageSender* pResourceUsageSender;

	UPROPERTY()
	UProjectTwoGameInstance* PT_GameInstatnce;
};
