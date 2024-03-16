// Fill out your copyright notice in the Description page of Project Settings.


#include "Authorize/ServerVerifier.h"

UServerVerifier::UServerVerifier()
{
	UE_LOG(LogTemp, Log, TEXT("UServerVerifier::UServerVerifier()"));
}

UServerVerifier::~UServerVerifier()
{
	UE_LOG(LogTemp, Log, TEXT("UServerVerifier::~UServerVerifier()"));
}

void UServerVerifier::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UServerVerifier::Initialize()"));
}

void UServerVerifier::Start(UGameInstance* GameInstance)
{
	Super::Start(GameInstance);

#ifdef UE_BUILD_SHIPPING
	TArray<FString> CommandLineTokens;
	TArray<FString> CommandLineSwitches;
	FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);
	for(FString str: CommandLineSwitches) {
		FString key, value;
		if(str.Split("=", &key, &value)) {
			if(key.Equals("password"))	{
				ServerPassword = value;
			}
		}
	}
#else
	ServerPassword = "WhaleTekServerPasswordTest";
#endif
	
	UE_LOG(LogTemp, Log, TEXT("UServerVerifier::Start()"));
}

void UServerVerifier::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("UServerVerifier::Shutdown()"));
	Super::Shutdown();
}

FString UServerVerifier::GetServerPassword() const
{
	return ServerPassword;
}
