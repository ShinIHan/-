// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "APIGateway.h"
#include "CognitoAPIGateway.generated.h"

// ExchangeCodeForTokens
DECLARE_DYNAMIC_DELEGATE(FExchangeCodeForTokensSuccessEvent);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExchangeCodeForTokensFailureEvent, FString, FailureReason);

/**
 * 
 */
UCLASS(Blueprintable)
class WHALETEKAWS_API UCognitoAPIGateway : public UAPIGateway
{
	GENERATED_BODY()

private:
	void OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FExchangeCodeForTokensSuccessEvent SuccessDelegate, FExchangeCodeForTokensFailureEvent FailureDelegate);
	void OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	UCognitoAPIGateway();
	virtual ~UCognitoAPIGateway() override;

	virtual void Initialize() override;
	virtual void Start(UAWSSubsystem* pSubsystem) override;
	virtual void Shutdown() override;

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Request")
		bool SendExchangeCodeForTokensRequest(FText ChangedUrl, FExchangeCodeForTokensSuccessEvent SuccessDelegate, FExchangeCodeForTokensFailureEvent FailureDelegate);
	
	UFUNCTION()
		void SendRetrieveNewTokensRequest();

	UFUNCTION()
		bool SendInvalidateTokensRequest();
};
