// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Networking.h"
#include "Delegates.h"
#include "NetworkTCPHelper.generated.h"

UCLASS()
class SERVERMANAGERCONNECTOR_API UNetworkTCPHelper : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

protected:
	FSocket* pSocket;

	TSharedPtr<FInternetAddr> pInetAddr;
	FString			IpAddress;
	int32			Port;

	bool			bIsConnected;
	bool			bIsTicking;

	UPROPERTY()
	UGameInstance* pGameInstance;

	UPROPERTY()
	FString			PlayerIdOrMaxAcceptablePlayers;

//public:
//	FConnectSuccessEvent	ConnectSuccessEvent;

private:
	void Receiver();
	void ProcessData(uint8* buf, size_t ioByte);
	virtual void ProcessSocketMessage(char* ptr);
	virtual void SendConnectPacket();

protected:
	virtual void InitializeSocket();
	void Disconnect();
	void SendPacket(void* packet);
	FSocket* GetSocket() { return pSocket; }

public:
	UNetworkTCPHelper();
	virtual ~UNetworkTCPHelper() override;
	virtual void Tick(float deltaTime);

	void SetIPAddressAndPortNumber(FString ip, int32 port);

	//void Start(UGameInstance* gameInstance, FString playerIdOrMaxAcceptablePlayers, FConnectSuccessEvent connectSuccessDelegate);
	void Start(UGameInstance* gameInstance, FString playerIdOrMaxAcceptablePlayers);
	void Stop();

	// Tickable
	virtual ETickableTickType GetTickableTickType() const { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const { return true; }
	virtual bool IsTickableInEditor() const { return false; }
	virtual bool IsTickableWhenPaused() const { return false; }
	virtual TStatId GetStatId() const { return TStatId(); }
	virtual UWorld* GetTickableGameObjectWorld() const { return GetOuter()->GetWorld(); }
};
