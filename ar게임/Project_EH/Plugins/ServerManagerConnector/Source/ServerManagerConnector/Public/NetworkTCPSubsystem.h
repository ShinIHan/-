// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkTCPHelper.h"
#include "Delegates.h"
#include "NetworkTCPSubsystem.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_DELEGATE_OneParam(FMatchSuccessEvent, FString, LevelName);
DECLARE_DYNAMIC_DELEGATE(FMatchFailureEvent);
DECLARE_DYNAMIC_DELEGATE(FConnectSuccessEvent);
DECLARE_DYNAMIC_DELEGATE(FConnectFailureEvent);

UCLASS()
class SERVERMANAGERCONNECTOR_API UNetworkTCPSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
		UNetworkTCPHelper* pNetworkHelper;

public:
	FConnectSuccessEvent ConnectSuccessEvent;
	
	FConnectFailureEvent ConnectFailureEvent;

	FMatchSuccessEvent MatchSuccessEvent;

	FMatchFailureEvent MatchFailureEvent;

public:
	UNetworkTCPSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Custom Tickable
	UFUNCTION(BlueprintCallable)
		bool StartNetworkHelper(FString PlayerIdOrMaxAcceptablePlayers, FConnectSuccessEvent ConnectSuccessDelegate, FConnectFailureEvent ConnectFailureDelegate);

	UFUNCTION(BlueprintCallable)
		bool StopNetworkHelper();

	// Server
	UFUNCTION(BlueprintCallable)
		bool SendConnectPlayerPacket();

	UFUNCTION(BlueprintCallable)
		bool SendDisconnectPlayerPacket();

	UFUNCTION(BlueprintCallable)
		bool SendEventRequestPacket(int eventType);

	// Client
	UFUNCTION(BlueprintCallable)
		bool SendMatchRequestPacket(FMatchSuccessEvent MatchSuccessDelegate, FMatchFailureEvent MatchFailureDelegate, FString Classroom);
};
