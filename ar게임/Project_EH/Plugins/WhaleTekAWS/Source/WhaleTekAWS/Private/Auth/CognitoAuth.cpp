// Fill out your copyright notice in the Description page of Project Settings.

#include "Auth/CognitoAuth.h"
#include "AWSSubsystem.h"
#include "IWebBrowserCookieManager.h"
#include "IWebBrowserSingleton.h"
#include "WebBrowserModule.h"
#include "AWSFunctionLibrary.h"


UCognitoAuth::UCognitoAuth()
{
	LoginUrl = UAWSFunctionLibrary::ReadFile("Urls/LoginUrl.txt");
	CallbackUrl = UAWSFunctionLibrary::ReadFile("Urls/CallbackUrl.txt");

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::UCognitoAuth()"));
}

UCognitoAuth::~UCognitoAuth()
{
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::~UCognitoAuth()"));
}

void UCognitoAuth::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::Initialize()"));
}

void UCognitoAuth::Start(UAWSSubsystem* pSubsystem)
{
	Super::Start(pSubsystem);
	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::Start()"));
}

void UCognitoAuth::Shutdown()
{
	pAWSSubsystem->GetGameInstance()->GetWorld()->GetTimerManager().ClearTimer(RetrieveNewTokensHandle);

	if (Token.Len() > 0)
		pAWSSubsystem->GetCognitoGateway()->SendInvalidateTokensRequest();

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::Shutdown()"));

	Super::Shutdown();
}

void UCognitoAuth::SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken)
{
	Token = NewAccessToken;
	IdToken = NewIdToken;
	RefreshToken = NewRefreshToken;

	pAWSSubsystem->GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(RetrieveNewTokensHandle, pAWSSubsystem->GetCognitoGateway(), &UCognitoAPIGateway::SendRetrieveNewTokensRequest, 1.0f, false, 3300.f);

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::SetCognitoTokens(), Access token: %s, Id token: %s, Refresh token: %s"), *Token, *IdToken, *RefreshToken);
}

FString UCognitoAuth::GetLoginUrlAndClearCookie()
{
	IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();

	if (WebBrowserSingleton != nullptr) {
		const TOptional<FString> DefaultContext;
		const TSharedPtr<IWebBrowserCookieManager> CookieManager = WebBrowserSingleton->GetCookieManager(DefaultContext);
		if (CookieManager.IsValid()) {
			CookieManager->DeleteCookies();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UCognitoAuth::GetLoginUrlAndClearCookie(), LoginUrl: %s"), *LoginUrl);
	return LoginUrl;
}

FString UCognitoAuth::GetCallbackUrl() const
{
	return CallbackUrl;
}

FString UCognitoAuth::GetRefreshToken() const
{
	return RefreshToken;
}