// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "Networking.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "pdh.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "ResourceUsageSender.generated.h"

#pragma comment(lib, "pdh.lib")

#pragma pack(push, 1)
struct sc_resource_usage_packet {
	double cpuPercentage;
	double memPercentage;
};
#pragma pack(pop)

UCLASS()
class PROJECT_TWO_API UResourceUsageSender : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

private:
	FSocket* pListenSocket;
	FSocket* pClientSocket;

	TSharedPtr<FInternetAddr>	pInetAddr;

	UPROPERTY()
		int32			Port;

	bool			bIsAccepted;

	bool			bIsTicking;

	UPROPERTY()
		FTimerHandle	SendResourceUsageHandle;

	UPROPERTY()
		UGameInstance* pGameInstance;

	// CPU
	PDH_HQUERY		m_cpuQuery;
	PDH_HCOUNTER	m_cpuTotal;

	// Memory
	MEMORYSTATUSEX	m_memInfo;

private:
	bool InitializeSocket();
	void Disconnect();
	void SendResourceUsage();

public:
	UResourceUsageSender();
	virtual ~UResourceUsageSender() override;

	virtual void Tick(float deltaTime);

	void Start(UGameInstance* gameInstance);
	void Stop();

	// Resources
	void InitializeCpuUsage();
	double GetCpuUsage();
	double GetMemoryUsage();

	// Tickable
	virtual ETickableTickType GetTickableTickType() const { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const { return true; }
	virtual bool IsTickableInEditor() const { return false; }
	virtual bool IsTickableWhenPaused() const { return false; }
	virtual TStatId GetStatId() const { return TStatId(); }
	virtual UWorld* GetTickableGameObjectWorld() const { return GetOuter()->GetWorld(); }
};
