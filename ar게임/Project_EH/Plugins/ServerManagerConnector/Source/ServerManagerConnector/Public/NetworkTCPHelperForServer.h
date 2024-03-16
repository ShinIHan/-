// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkTCPHelper.h"
#include "Delegates.h"
#include "NetworkTCPHelperForServer.generated.h"

/**
 * 
 */

UCLASS()
class SERVERMANAGERCONNECTOR_API UNetworkTCPHelperForServer : public UNetworkTCPHelper
{
	GENERATED_BODY()
	
private:
	void InitializeSocket();
	virtual void ProcessSocketMessage(char* ptr) override;
	virtual void SendConnectPacket() override;

public:
	UNetworkTCPHelperForServer();

	UFUNCTION()
		void SendConnectPlayerPacket();

	UFUNCTION()
		void SendDisconnectPlayerPacket();

	UFUNCTION()
		void SendEventRequestPacket(int eventType);
};
