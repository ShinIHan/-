// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkTCPHelperForClient.h"
#include <string>
#include "Kismet/KismetSystemLibrary.h"
#include "NetworkCallbackInterface.h"
#include "Packet.h"
#include "NetworkTCPSubsystem.h"

UNetworkTCPHelperForClient::UNetworkTCPHelperForClient()
{

}

void UNetworkTCPHelperForClient::InitializeSocket()
{
	//SetIPAddressAndPortNumber("3.36.1.247", PORT);//¸ÞÅ¸Ä·
	SetIPAddressAndPortNumber("43.200.188.87", PORT);
	//SetIPAddressAndPortNumber("127.0.0.1", PORT);
	UNetworkTCPHelper::InitializeSocket();
}

void UNetworkTCPHelperForClient::ProcessSocketMessage(char* ptr)
{
	switch (ptr[1]) {
	case SM2C_CONNECT:
	{
		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForClient::ProcessSocketMessage() - Received connect packet"));
	}
	break;
	case SM2C_DISCONNECT:
	{
		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForClient::ProcessSocketMessage() - Received disconnect packet"));
		Disconnect();
	}
	break;
	case SM2C_MATCHRESPONSE:
	{
		sm2c_matchresponse_packet* packet = reinterpret_cast<sm2c_matchresponse_packet*>(ptr);
		FString ip = packet->ip;
		if (ip.Len() > 0) {
			FString levelName = ip + ":7777";
			//MatchSuccessEvent.ExecuteIfBound(levelName);
			pGameInstance->GetSubsystem<UNetworkTCPSubsystem>()->MatchSuccessEvent.ExecuteIfBound(levelName);
		}
		else {
			//MatchFailureEvent.ExecuteIfBound();
			pGameInstance->GetSubsystem<UNetworkTCPSubsystem>()->MatchFailureEvent.ExecuteIfBound();
		}

		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForClient::ProcessSocketMessage() - Received match response packet"));
	}
	break;
	default:
	{
		FString packetInfo = ptr;
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelperForClient::ProcessSocketMessage() Error - Unknown packet: %s"), *packetInfo);
		Disconnect();
		UKismetSystemLibrary::QuitGame(pGameInstance->GetWorld(), nullptr, EQuitPreference::Quit, true);
	}
	break;
	}
}

void UNetworkTCPHelperForClient::SendConnectPacket()
{
	c2sm_connect_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = C2SM_CONNECT;
	std::string playerIdStr = TCHAR_TO_ANSI(*PlayerIdOrMaxAcceptablePlayers);
	std::wstring playerIdWstr;
	playerIdWstr.assign(playerIdStr.begin(), playerIdStr.end());
	wcscpy_s(packet.player_sub, playerIdWstr.c_str());

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForClient::SendConnectPacket()"));

	SendPacket(&packet);
}

bool UNetworkTCPHelperForClient::SendMatchRequestPacket(FString Classroom)
{
	if (GetSocket()->GetConnectionState() != ESocketConnectionState::SCS_Connected) {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelperForClient::SendMatchRequestPacket() Error - Socket state is not connected"));
		return false;
	}

	c2sm_matchrequest_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = C2SM_MATCHREQUEST;
	std::string sClassroom = TCHAR_TO_ANSI(*Classroom);
	std::wstring wsClassroom;
	wsClassroom.assign(sClassroom.begin(), sClassroom.end());
	wcscpy_s(packet.classroom, wsClassroom.c_str());

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForClient::SendMatchRequestPacket()"));

	SendPacket(&packet);
	return true;
}