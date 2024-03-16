// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Auth.h"
#include "CognitoAuth.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WHALETEKAWS_API UCognitoAuth : public UAuth
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
		FString LoginUrl;

	UPROPERTY()
		FString CallbackUrl;

	UPROPERTY()
		FString IdToken;

	UPROPERTY()
		FString RefreshToken;

public:
	UPROPERTY()
		FTimerHandle RetrieveNewTokensHandle;

public:
	UCognitoAuth();
	virtual ~UCognitoAuth() override;

	virtual void Initialize() override;
	virtual void Start(UAWSSubsystem* pSubsystem);
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable)
		FString GetLoginUrlAndClearCookie();

	UFUNCTION()
		FString GetCallbackUrl() const;

	UFUNCTION()
		FString GetRefreshToken() const;

	UFUNCTION()
	void SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken);
};
