// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Authorizer.h"
#include "Interfaces/IHttpRequest.h"
#include "CognitoAuthorizer.generated.h"

/**
 * 
 */
UCLASS()
class AWSCONNECTOR_API UCognitoAuthorizer : public UAuthorizer
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FString LoginUrl;

	UPROPERTY()
	FString CallbackUrl;

	UPROPERTY()
	FString AccessToken;

	UPROPERTY()
	FString IdToken;

	UPROPERTY()
	FString RefreshToken;

	UPROPERTY()
	FTimerHandle RetrieveNewTokensHandle;

private:
	void SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken);
	void SendRetrieveNewTokensRequest();
	bool SendInvalidateTokensRequest();

	void OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
public:
	UCognitoAuthorizer();
	virtual ~UCognitoAuthorizer() override;

	virtual void Initialize() override;
	virtual void Start(UGameInstance* GameInstance) override;
	virtual void Shutdown() override;
	
	UFUNCTION()
	FString GetLoginUrlAndClearCookie();

	UFUNCTION()
	bool SendExchangeCodeForTokensRequest(FText ChangedUrl);

	UFUNCTION()
	FString GetAccessToken() const;
};
