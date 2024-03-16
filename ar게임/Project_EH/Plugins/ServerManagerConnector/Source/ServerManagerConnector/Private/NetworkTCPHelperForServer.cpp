// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkTCPHelperForServer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameMode.h"
#include "NetworkCallbackInterface.h"
#include "Packet.h"

UNetworkTCPHelperForServer::UNetworkTCPHelperForServer()
{
}

void UNetworkTCPHelperForServer::InitializeSocket()
{
	//SetIPAddressAndPortNumber("3.36.1.247", PORT);
	SetIPAddressAndPortNumber("43.200.188.87", PORT);
	//SetIPAddressAndPortNumber("127.0.0.1", PORT);
	UNetworkTCPHelper::InitializeSocket();
}

void UNetworkTCPHelperForServer::ProcessSocketMessage(char* ptr)
{
	switch (ptr[1]) {
	case SM2S_CONNECT:
	{
		sm2s_connect_packet* packet = reinterpret_cast<sm2s_connect_packet*>(ptr);
		FString vivoxChannelName = packet->vivox_channel_name;
		if (pGameInstance->GetClass()->ImplementsInterface(UNetworkCallbackInterface::StaticClass()))
			INetworkCallbackInterface::Execute_ReceivedVivoxChannelNameEvent(pGameInstance, vivoxChannelName);

		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received connect packet, Vivox channel name : %s"), *vivoxChannelName);
	}
	break;
	case SM2S_DISCONNECT:
	{
		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received disconnect packet"));
		Disconnect();
	}
	break;
	//case SM2S_LECTUREEVENT:
	//{
	//	sm2s_lecture_event_packet* packet = reinterpret_cast<sm2s_lecture_event_packet*>(ptr);
	//	FString classroom = packet->classroom;
	//	FString lectureId = packet->lectureId;
	//	FString content = packet->content;
	//	FDateTime startDateTime(packet->start_year, packet->start_month, packet->start_day, packet->start_hour, packet->start_minute, 0);
	//	FDateTime endDateTime(packet->end_year, packet->end_month, packet->end_day, packet->end_hour, packet->end_minute, 0);
	//
	//	if (pGameInstance->GetClass()->ImplementsInterface(UNetworkCallbackInterface::StaticClass()))
	//		INetworkCallbackInterface::Execute_ReceivedLectureEvent(pGameInstance, startDateTime, endDateTime, classroom, lectureId, content);
	//
	//	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received lecture event packet, end time: %d/%d/%d-%d:%d, classroom : %s, lectureId : %s, content : %s"),
	//		packet->end_year, packet->end_month, packet->end_day, packet->end_hour, packet->end_minute, *classroom, *lectureId, *content);
	//}
	//break;
	//case SM2S_INVITEEVENT:
	//{
	//	sm2s_invite_event_packet* packet = reinterpret_cast<sm2s_invite_event_packet*>(ptr);
	//	FString classroom = packet->classroom;
	//	FString lectureId = packet->lectureId;
	//	FString content = packet->content;
	//	FDateTime dateTime(packet->start_year, packet->start_month, packet->start_day, packet->start_hour, packet->start_minute, 0);
	//
	//	if (pGameInstance->GetClass()->ImplementsInterface(UNetworkCallbackInterface::StaticClass()))
	//		INetworkCallbackInterface::Execute_ReceivedInviteEvent(pGameInstance, dateTime, classroom, lectureId, content);
	//
	//	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received invite event packet, lecture start time: %d/%d/%d-%d:%d, classroom : %s, lectureId : %s, content : %s"),
	//		packet->start_year, packet->start_month, packet->start_day, packet->start_hour, packet->start_minute, *classroom, *lectureId, *content);
	//}
	//break;
	case SM2S_EVENTRESPONSE:
	{
		sm2s_eventresponse_packet* packet = reinterpret_cast<sm2s_eventresponse_packet*>(ptr);
		if (pGameInstance->GetClass()->ImplementsInterface(UNetworkCallbackInterface::StaticClass()))
			INetworkCallbackInterface::Execute_ReceivedEventResponseEvent(pGameInstance, packet->event_type);
	
		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received event response packet, event type : %d"), packet->event_type);
	}
	break;
	case SM2S_DISCONNECTPLAYERREQUEST:
	{
		sm2s_disconnectplayerrequest_packet* packet = reinterpret_cast<sm2s_disconnectplayerrequest_packet*>(ptr);
		FString playerSub = packet->player_sub;
		if (pGameInstance->GetClass()->ImplementsInterface(UNetworkCallbackInterface::StaticClass()))
			INetworkCallbackInterface::Execute_ReceivedDisconnectPlayerRequestEvent(pGameInstance, playerSub);

		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() - Received disconnect player request packet, player sub : %s"), *playerSub);
	}
	break;
	default:
	{
		FString packetInfo = ptr;
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPHelperForServer::ProcessSocketMessage() Error - Unknown packet: %s"), *packetInfo);
		Disconnect();
		UKismetSystemLibrary::QuitGame(pGameInstance->GetWorld(), nullptr, EQuitPreference::Quit, true);
	}
	break;
	}
}

void UNetworkTCPHelperForServer::SendConnectPacket()
{
	s2sm_connect_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = S2SM_CONNECT;
	packet.connected_players = 0;
	packet.max_acceptable_players = FCString::Atoi(*PlayerIdOrMaxAcceptablePlayers);

	auto gameMode = UGameplayStatics::GetGameMode(pGameInstance->GetWorld());
	if (gameMode != nullptr) {
		packet.connected_players = gameMode->GetNumPlayers();
	}

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::SendConnectPacket() - connected players: %d, max acceptable players: %d"), packet.connected_players, packet.max_acceptable_players);

	SendPacket(&packet);
}

void UNetworkTCPHelperForServer::SendConnectPlayerPacket()
{
	s2sm_connect_player_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = S2SM_CONNECTPLAYER;
	packet.connected_players = 0;

	auto gameMode = UGameplayStatics::GetGameMode(pGameInstance->GetWorld());
	if (gameMode != nullptr) {
		packet.connected_players = gameMode->GetNumPlayers();
	}

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::SendConnectPlayerPacket()"));

	SendPacket(&packet);
}

void UNetworkTCPHelperForServer::SendDisconnectPlayerPacket()
{
	s2sm_disconnect_player_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = S2SM_DISCONNECTPLAYER;
	packet.connected_players = 0;

	auto gameMode = UGameplayStatics::GetGameMode(pGameInstance->GetWorld());
	if (gameMode != nullptr) {
		packet.connected_players = gameMode->GetNumPlayers();
	}

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::SendDisconnectPlayerPacket()"));

	SendPacket(&packet);
}

void UNetworkTCPHelperForServer::SendEventRequestPacket(int eventType)
{
	s2sm_eventrequest_packet packet;
	unsigned char size = static_cast<unsigned char>(sizeof(packet));
	packet.size = size;
	packet.packet_type = S2SM_EVENTREQUEST;
	packet.event_type = eventType;

	UE_LOG(LogTemp, Log, TEXT("UNetworkTCPHelperForServer::SendEventRequestPacket() - event type: %d"), eventType);

	SendPacket(&packet);
}