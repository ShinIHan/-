// Fill out your copyright notice in the Description page of Project Settings.


#include "Auth/ServerAuth.h"

UServerAuth::UServerAuth()
{
	UE_LOG(LogTemp, Log, TEXT("UServerAuth::UServerAuth()"));
}

UServerAuth::~UServerAuth()
{
	UE_LOG(LogTemp, Log, TEXT("UServerAuth::~UServerAuth()"));
}

void UServerAuth::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UServerAuth::UServerAuth()"));
}

void UServerAuth::Start(UAWSSubsystem* pSubsystem)
{
	Super::Start(pSubsystem);

#ifdef WITH_EDITOR
	Token = "WhaleTekServerPasswordTest";
#else
	TArray<FString> CommandLineTokens;
	TArray<FString> CommandLineSwitches;
	FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);
	for (FString str : CommandLineSwitches) {
		FString key, value;
		if (str.Split("=", &key, &value)) {
			if (key.Equals("password")) {
				Token = value;
			}
		}
	}
#endif

	UE_LOG(LogTemp, Log, TEXT("UServerAuth::Start()"));
}

void UServerAuth::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("UServerAuth::Shutdown()"));
	Super::Shutdown();
}