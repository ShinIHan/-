// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkTCPSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NetworkTCPHelperForServer.h"
#include "NetworkTCPHelperForClient.h"

#define BUFSIZE		512

UNetworkTCPSubsystem::UNetworkTCPSubsystem()
{
}

void UNetworkTCPSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

}

void UNetworkTCPSubsystem::Deinitialize()
{
	Super::Deinitialize();

}

bool UNetworkTCPSubsystem::StartNetworkHelper(FString PlayerIdOrMaxAcceptablePlayers, FConnectSuccessEvent ConnectSuccessDelegate, FConnectFailureEvent ConnectFailureDelegate)
{
	if (pNetworkHelper != nullptr)
	{
		return true;
		//StopNetworkHelper();
	}

	if (true == GetGameInstance()->IsDedicatedServerInstance()) {
		pNetworkHelper = NewObject<UNetworkTCPHelperForServer>(GetTransientPackage(), UNetworkTCPHelperForServer::StaticClass());
	}
	else {
		pNetworkHelper = NewObject<UNetworkTCPHelperForClient>(GetTransientPackage(), UNetworkTCPHelperForClient::StaticClass());
	}

	if (pNetworkHelper != nullptr) {
		pNetworkHelper->AddToRoot();
		//pNetworkHelper->Start(GetGameInstance(), PlayerIdOrMaxAcceptablePlayers, ConnectSuccessDelegate);
		pNetworkHelper->Start(GetGameInstance(), PlayerIdOrMaxAcceptablePlayers);
		ConnectSuccessEvent = ConnectSuccessDelegate;
		ConnectFailureEvent = ConnectFailureDelegate;

		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::StartNetworkHelper() - Network tick start complete"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::StartNetworkHelper() Error - pNetworkHelper is nullptr"));
		return false;
	}
}

bool UNetworkTCPSubsystem::StopNetworkHelper()
{
	if (pNetworkHelper != nullptr) {
		pNetworkHelper->Stop();
		pNetworkHelper->RemoveFromRoot();
		ConnectSuccessEvent.Unbind();

		UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::StopNetworkHelper() - Network tick stop complete"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::StopNetworkHelper() Error - pNetworkHelper is nullptr"));
		return false;
	}
}

bool UNetworkTCPSubsystem::SendConnectPlayerPacket()
{
	if (pNetworkHelper != nullptr) {
		auto pServerHelper = Cast<UNetworkTCPHelperForServer>(pNetworkHelper);
		if (pServerHelper != nullptr) {
			pServerHelper->SendConnectPlayerPacket();
			UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::SendConnectPlayerPacket() complete"));
			return true;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendConnectPlayerPacket() Error - pNetworkHelper cast failed"));
			return false;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendConnectPlayerPacket() Error - pNetworkHelper is nullptr"));
		return false;
	}
}

bool UNetworkTCPSubsystem::SendDisconnectPlayerPacket()
{
	if (pNetworkHelper != nullptr) {
		auto pServerHelper = Cast<UNetworkTCPHelperForServer>(pNetworkHelper);
		if (pServerHelper != nullptr) {
			pServerHelper->SendDisconnectPlayerPacket();
			UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::SendDisconnectPlayerPacket() complete"));
			return true;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendDisconnectPlayerPacket() Error - pNetworkHelper cast failed"));
			return false;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendDisconnectPlayerPacket() Error - pNetworkHelper is nullptr"));
		return false;
	}
}

bool UNetworkTCPSubsystem::SendEventRequestPacket(int eventType)
{
	if (pNetworkHelper != nullptr) {
		auto pServerHelper = Cast<UNetworkTCPHelperForServer>(pNetworkHelper);
		if (pServerHelper != nullptr) {
			pServerHelper->SendEventRequestPacket(eventType);
			UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::SendEventRequestPacket() complete"));
			return true;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendEventRequestPacket() Error - pNetworkHelper cast failed"));
			return false;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendEventRequestPacket() Error - pNetworkHelper is nullptr"));
		return false;
	}
}

bool UNetworkTCPSubsystem::SendMatchRequestPacket(FMatchSuccessEvent MatchSuccessDelegate, FMatchFailureEvent MatchFailureDelegate, FString Classroom)
{
	if (pNetworkHelper != nullptr) {
		auto pClientHelper = Cast<UNetworkTCPHelperForClient>(pNetworkHelper);
		if (pClientHelper != nullptr) {
			if (true == pClientHelper->SendMatchRequestPacket(Classroom)) {
				UE_LOG(LogTemp, Log, TEXT("UNetworkTCPSubsystem::SendMatchRequestPacket() complete"));
				//pClientHelper->MatchSuccessEvent = MatchSuccessDelegate;
				MatchSuccessEvent = MatchSuccessDelegate;
				MatchFailureEvent = MatchFailureDelegate;
				return true;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendMatchRequestPacket() Error - pNetworkHelper cast failed"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendMatchRequestPacket() Error - pNetworkHelper cast failed"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UNetworkTCPSubsystem::SendMatchRequestPacket() Error - pNetworkHelper is nullptr"));
	}

	MatchFailureDelegate.ExecuteIfBound();
	return false;
}
