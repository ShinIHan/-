// Fill out your copyright notice in the Description page of Project Settings.


#include "Authorize/Authorizer.h"
#include "AWSBlueprintFunctionLibrary.h"

UAuthorizer::UAuthorizer()
{
	ApiUrl = UAWSBlueprintFunctionLibrary::ReadFile("Urls/ApiUrl.txt");
	pHttpModule = &FHttpModule::Get();
}

UAuthorizer::~UAuthorizer()
{

}

void UAuthorizer::Initialize()
{
	
}

void UAuthorizer::Start(UGameInstance* GameInstance)
{
	pGameInstance = GameInstance;
}

void UAuthorizer::Shutdown()
{

}

FString UAuthorizer::GetApiUrl() const
{
	return ApiUrl;
}

FHttpModule* UAuthorizer::GetHttpModule()
{
	return pHttpModule;
}
