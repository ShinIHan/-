// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/ResourceUsageSender.h"
#include <Psapi.h>
#include "GenericPlatform/GenericPlatformTime.h"

#define BUFSIZE		sizeof(sc_resource_usage_packet)

UResourceUsageSender::UResourceUsageSender()
{
	InitializeCpuUsage();
	m_memInfo.dwLength = sizeof(MEMORYSTATUSEX);

	Port = 9000;

	bIsAccepted = false;
	bIsTicking = false;

	UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::UResourceUsageSender() Complete"));
}

UResourceUsageSender::~UResourceUsageSender()
{
	UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::~UResourceUsageSender()"));
}

void UResourceUsageSender::Tick(float deltaTime)
{
	if (pGameInstance == nullptr)
		return;

	if (true == bIsTicking) {
		if (pListenSocket != nullptr)
		{
			if (bIsAccepted == true)
			{
				bool pending;
				if (pClientSocket->HasPendingConnection(pending))
				{
					if (true == pending)
					{
						UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Tick() - Socket has pending connection"));
						Disconnect();
					}
				}
			}
			else
			{
				pClientSocket = pListenSocket->Accept("ResourceUsageSender");
				if (pClientSocket != nullptr)
				{
					bIsAccepted = true;
					UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Tick() - Socket has accepted"));
					pGameInstance->GetWorld()->GetTimerManager().SetTimer(SendResourceUsageHandle, this, &UResourceUsageSender::SendResourceUsage, 1.0f, true, 1.0f);
				}
			}
		}
		else{
			UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Tick() - listenSocket is nullptr"));
			InitializeSocket();		
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Tick() - ticking is false"));
	}
}

void UResourceUsageSender::Start(UGameInstance* gameInstance)
{
	pGameInstance = gameInstance;
	bIsTicking = true;

	InitializeSocket();

	UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Start()"));
}

void UResourceUsageSender::Stop()
{
	Disconnect();

	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pListenSocket);
	pListenSocket = nullptr;

	bIsTicking = false;

	UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Stop()"));
}

bool UResourceUsageSender::InitializeSocket()
{
	pInetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	pInetAddr->SetAnyAddress();
#pragma push_macro("SetPort")
#undef SetPort
	pInetAddr->SetPort(Port);
#pragma pop_macro("SetPort")

	if (pListenSocket != nullptr) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pListenSocket);
		pListenSocket = nullptr;
		UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::InitializeSocket() - listenSocket is not nullptr, destroy old socket..."));
	}

	pListenSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, "ResourceUsageSender", false);
	int retval = pListenSocket->SetNonBlocking(true);
	if (false == retval)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pListenSocket);
		pListenSocket = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("UResourceUsageSender::InitializeSocket() - set nonblocking failed"));
		return false;
	}

	retval = pListenSocket->Bind(*pInetAddr);
	if (false == retval)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pListenSocket);
		pListenSocket = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("UResourceUsageSender::InitializeSocket() - bind failure"));
		return false;
	}

	retval = pListenSocket->Listen(5);
	if (false == retval)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pListenSocket);
		pListenSocket = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("UResourceUsageSender::InitializeSocket() - listen failure"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UResourceUsageSender::InitializeSocket() Complete, port: %d"), Port);
	return true;
}

void UResourceUsageSender::Disconnect()
{
	pGameInstance->GetWorld()->GetTimerManager().ClearTimer(SendResourceUsageHandle);

	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pClientSocket);
	pClientSocket = nullptr;
	bIsAccepted = false;

	UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::Disconnect()"));
}

void UResourceUsageSender::SendResourceUsage()
{
	if (pClientSocket == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("UResourceUsageSender::SendResourceUsage() - clientSocket is nullptr"));
		return;
	}

	sc_resource_usage_packet	packet;

	packet.cpuPercentage = GetCpuUsage();
	packet.memPercentage = GetMemoryUsage();

	uint8* p = reinterpret_cast<uint8*>(&packet);
	int32 sent = 0;

	int size = sizeof(sc_resource_usage_packet);
	int size2 = sizeof(uint8);
	int send_bytes = pClientSocket->Send(p, size, sent);
	if (send_bytes > 0) {
		UE_LOG(LogTemp, Log, TEXT("UResourceUsageSender::SendResourceUsage() Complete"));
	}
	else {
		Disconnect();
	}
}

void UResourceUsageSender::InitializeCpuUsage()
{
	PdhOpenQuery(NULL, NULL, &m_cpuQuery);
	PdhAddEnglishCounter(m_cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &m_cpuTotal);
	PdhCollectQueryData(m_cpuQuery);
}

double UResourceUsageSender::GetCpuUsage()
{
	PDH_FMT_COUNTERVALUE counterVal = { 0 };
	PdhCollectQueryData(m_cpuQuery);
	PdhGetFormattedCounterValue(m_cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}

double UResourceUsageSender::GetMemoryUsage()
{
	GlobalMemoryStatusEx(&m_memInfo);
	DWORDLONG physMemUsed = m_memInfo.dwMemoryLoad;
	return (double)physMemUsed;
}