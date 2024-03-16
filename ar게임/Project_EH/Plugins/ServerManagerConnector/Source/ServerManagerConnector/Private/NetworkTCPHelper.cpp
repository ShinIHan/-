// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkTCPHelper.h"
#include "Kismet/GameplayStatics.h"
#include "NetworkTCPSubsystem.h"

#define BUFSIZE		512

UNetworkTCPHelper::UNetworkTCPHelper()
{
	bIsConnected = false;
	bIsTicking = false;
}

UNetworkTCPHelper::~UNetworkTCPHelper()
{
	Stop();
}

void UNetworkTCPHelper::Tick(float deltaTime)
{
	if (pGameInstance == nullptr)
		return;

	if (true == bIsTicking) {
		if (pSocket != nullptr)
		{
			switch (pSocket->GetConnectionState()) {
			case ESocketConnectionState::SCS_NotConnected:
			{
				bIsConnected = false;
				pSocket->Connect(*pInetAddr);
			}
			break;
			case ESocketConnectionState::SCS_Connected:
			{
				if (false == bIsConnected) {
					if (true == pGameInstance->IsDedicatedServerInstance()) {
						if (UGameplayStatics::GetGameMode(pGameInstance->GetWorld()) != nullptr) {
							SendConnectPacket();
							bIsConnected = true;

							UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Tick() - Connection state is Connected"));
						}
					}
					else {
						SendConnectPacket();
						bIsConnected = true;

						UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Tick() - Connection state is Connected"));
					}
					//ConnectSuccessEvent.ExecuteIfBound();
					pGameInstance->GetSubsystem<UNetworkTCPSubsystem>()->ConnectSuccessEvent.ExecuteIfBound();
				}
				Receiver();
			}
			break;
			case ESocketConnectionState::SCS_ConnectionError:
			{
				UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Tick() - Connection state is Connection Error"));

				Disconnect();
				pGameInstance->GetSubsystem<UNetworkTCPSubsystem>()->ConnectFailureEvent.ExecuteIfBound();
				InitializeSocket();
			}
			break;
			default:
			{
			}
			break;
			}

			if (pSocket != nullptr)
			{
				bool Pending;
				if (pSocket->HasPendingConnection(Pending)) {
					if (true == Pending) {
						UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Tick() - Socket has pending connection"));

						Disconnect();
						InitializeSocket();
					}
				}
			}
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Tick() - Socket is nullptr"));

			InitializeSocket();
			bIsConnected = false;
		}
	}
}

//void UNetworkTCPHelper::Start(UGameInstance* gameInstance, FString playerIdOrMaxAcceptablePlayers, FConnectSuccessEvent connectSuccessDelegate)
//{
//	pGameInstance = gameInstance;
//	PlayerIdOrMaxAcceptablePlayers = playerIdOrMaxAcceptablePlayers;
//	bIsTicking = true;
//
//	ConnectSuccessEvent = connectSuccessDelegate;
//
//	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Start() - Network TCP Helper start"));
//}

void UNetworkTCPHelper::Start(UGameInstance* gameInstance, FString playerIdOrMaxAcceptablePlayers)
{
	pGameInstance = gameInstance;
	PlayerIdOrMaxAcceptablePlayers = playerIdOrMaxAcceptablePlayers;
	bIsTicking = true;

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Start() - Network TCP Helper start"));
}

void UNetworkTCPHelper::Stop()
{
	Disconnect();
	bIsTicking = false;

	//ConnectSuccessEvent.Unbind();
	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Stop() - Network TCP Helper stop"));
}

void UNetworkTCPHelper::SetIPAddressAndPortNumber(FString ip, int32 port)
{
	IpAddress = ip;
	Port = port;
}

void UNetworkTCPHelper::InitializeSocket()
{
	pSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, IpAddress, false);
	pSocket->SetNonBlocking(true);

	FIPv4Address ip;
	FIPv4Address::Parse(IpAddress, ip);

	pInetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	pInetAddr->SetIp(ip.Value);
	pInetAddr->SetPort(Port);

	UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelper::InitializeSocket() - IP: %s, Port: %d"), *IpAddress, Port);
}

void UNetworkTCPHelper::SendPacket(void* packet)
{
	if (pSocket == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelper::SendPacket() - Send Failed, pSocket is nullptr"));
		return;
	}

	uint8* p = reinterpret_cast<uint8*>(packet);
	int32 sent = 0;
	int send_bytes = pSocket->Send(p, p[0], sent);
	if (send_bytes > 0) {
		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::SendPacket() - Send Complete"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelper::SendPacket() - Send Failed"));
	}
}

void UNetworkTCPHelper::Receiver()
{
	int32 BufferSize = BUFSIZE;
	int32 BytesRead = 0;
	uint8 Response[BUFSIZE];

	if (pSocket->Recv(Response, BufferSize, BytesRead)) {
		ProcessData(Response, BytesRead);
	}
}

void UNetworkTCPHelper::ProcessData(uint8* buf, size_t ioByte)
{
	static char packetBuffer[BUFSIZE];
	static int prevSize = 0;

	int restByte = ioByte;
	char* p = (char*)buf;
	int packetSize = 0;

	if (0 != prevSize) packetSize = (unsigned char)packetBuffer[0];
	while (restByte > 0) {
		if (0 == prevSize) packetSize = (unsigned char)*p;
		if (packetSize <= restByte + prevSize) {
			memcpy(packetBuffer + prevSize, p, packetSize - prevSize);
			p += packetSize - prevSize;
			restByte -= packetSize - prevSize;
			packetSize = 0;
			ProcessSocketMessage(packetBuffer);
			memset(packetBuffer, 0, BUFSIZE);
			prevSize = 0;
		}
		else {
			memcpy(packetBuffer + prevSize, p, restByte);
			prevSize += restByte;
			restByte = 0;
			p += restByte;
		}
	}
}

void UNetworkTCPHelper::ProcessSocketMessage(char* ptr)
{

}

void UNetworkTCPHelper::SendConnectPacket()
{

}

void UNetworkTCPHelper::Disconnect()
{
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(pSocket);
	pSocket = nullptr;
	bIsConnected = false;
	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelper::Disconnect()"));
}
	
