// Fill out your copyright notice in the Description page of Project Settings.


#include "Authorize/AWSAuthorizeSubsystem.h"

#include "Authorize/CognitoAuthorizer.h"
#include "Authorize/ServerVerifier.h"

UAWSAuthorizeSubsystem::UAWSAuthorizeSubsystem()
{
}

void UAWSAuthorizeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if(true == GetGameInstance()->IsDedicatedServerInstance()){
		pAuthorizer = NewObject<UServerVerifier>(GetTransientPackage(), UServerVerifier::StaticClass());
	}
	else{
		pAuthorizer = NewObject<UCognitoAuthorizer>(GetTransientPackage(), UCognitoAuthorizer::StaticClass());
	}
	pAuthorizer->AddToRoot();
	StartAuthorizer();
}

void UAWSAuthorizeSubsystem::Deinitialize()
{
	pAuthorizer->RemoveFromRoot();
	Super::Deinitialize();
}

bool UAWSAuthorizeSubsystem::StartAuthorizer()
{
	if(pAuthorizer != nullptr) {
		pAuthorizer->Start(GetGameInstance());
		UE_LOG(LogTemp, Log, TEXT("UAWSAuthorizeSubsystem::StartAuthorizer() Complete - Bind GameInstance into Authorizer"));
		return true;	
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UAWSAuthorizeSubsystem::StartAuthorizer() Error - Authorizer is nullptr"));
		return false;	
	}
}

FString UAWSAuthorizeSubsystem::GetLoginUrlAndClearCookie() const
{
	FString loginUrl;
	FString failureReason;
	
	if(pAuthorizer != nullptr) {
		const auto pCognitoAuthorizer = Cast<UCognitoAuthorizer>(pAuthorizer);
		if(pCognitoAuthorizer != nullptr){
			loginUrl = pCognitoAuthorizer->GetLoginUrlAndClearCookie();
			UE_LOG(LogTemp, Log, TEXT("UAWSAuthorizeSubsystem::GetLoginUrlAndClearCookie() Complete, LoginUrl: %s"), *loginUrl);
			return loginUrl;
		}
		else {
			failureReason = "pAuthorizer cast failed";
		}
	}
	else {
		failureReason = "pAuthorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UAWSAuthorizeSubsystem::GetLoginUrlAndClearCookie() Error, %s"), *failureReason);
	return "";
}

bool UAWSAuthorizeSubsystem::SendExchangeCodeForTokensRequest(FText changedUrl)
{
	FString failureReason;
	
	if(pAuthorizer != nullptr) {
		const auto pCognitoAuthorizer = Cast<UCognitoAuthorizer>(pAuthorizer);
		if(pCognitoAuthorizer != nullptr){
			const bool retval = pCognitoAuthorizer->SendExchangeCodeForTokensRequest(changedUrl);
			
			UE_LOG(LogTemp, Log, TEXT("UAWSAuthorizeSubsystem::SendExchangeCodeForTokensRequest() Complete, return value: %s"), retval ? TEXT("true") : TEXT("false"));
			return retval;
		}
		else {
			failureReason = "pAuthorizer cast failed";
		}
	}
	else {
		failureReason = "pAuthorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UAWSAuthorizeSubsystem::SendExchangeCodeForTokensRequest() Error, %s"), *failureReason);
	return false;
}

FString UAWSAuthorizeSubsystem::GetAuthorization() const
{
	FString failureReason;
	
	if(pAuthorizer != nullptr){
		const auto pServerVerifier = Cast<UServerVerifier>(pAuthorizer);
		const auto pCognitoAuthorizer = Cast<UCognitoAuthorizer>(pAuthorizer);
		if(pServerVerifier != nullptr)	{
			FString serverPassword = pServerVerifier->GetServerPassword();
			UE_LOG(LogTemp, Log, TEXT("UAWSAuthorizeSubsystem::GetAuthorization() Complete - return Server password"));
			return serverPassword;
		}
		else if(pCognitoAuthorizer != nullptr)	{
			FString accessToken = pCognitoAuthorizer->GetAccessToken();
			UE_LOG(LogTemp, Log, TEXT("UAWSAuthorizeSubsystem::GetAuthorization() Complete - return Client access token"));
			return accessToken;
		}
		else {
			failureReason = "pAuthorizer cast failed";
		}
	}
	else {
		failureReason = "pAuthorizer is nullptr";
	}

	UE_LOG(LogTemp, Warning, TEXT("UAWSAuthorizeSubsystem::GetAuthorization() Error, %s"), *failureReason);
	return "";
}
