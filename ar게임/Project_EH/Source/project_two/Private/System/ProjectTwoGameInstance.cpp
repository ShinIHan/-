// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProjectTwoGameInstance.h"


UProjectTwoGameInstance::UProjectTwoGameInstance()
{
}

void UProjectTwoGameInstance::Init()
{
	Super::Init();
}

void UProjectTwoGameInstance::Shutdown()
{
	if (Type == ENetworkType::TE_Cloud)
	{
		auto networkSubsystem = GetSubsystem<UNetworkTCPSubsystem>();
		if (networkSubsystem != nullptr)
		{
			networkSubsystem->StopNetworkHelper();
		}
	}
	
	Super::Shutdown();
}