// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkTCPHelper.h"
#include "Delegates.h"
#include "NetworkTCPHelperForClient.generated.h"

UCLASS()
class SERVERMANAGERCONNECTOR_API UNetworkTCPHelperForClient : public UNetworkTCPHelper
{
	GENERATED_BODY()
	
private:
	virtual void InitializeSocket() override;
	virtual void ProcessSocketMessage(char* ptr) override;
	virtual void SendConnectPacket() override;

public:
	//FMatchSuccessEvent MatchSuccessEvent;
	//
	//FMatchFailureEvent MatchFailureEvent;

public:
	UNetworkTCPHelperForClient();

	bool SendMatchRequestPacket(FString Classroom);
};
