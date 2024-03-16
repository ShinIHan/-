// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Authorizer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AWSAuthorizeSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AWSCONNECTOR_API UAWSAuthorizeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	UAuthorizer*	pAuthorizer;

public:
	UAWSAuthorizeSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	bool StartAuthorizer();

	// Client
	UFUNCTION(BlueprintCallable)
	FString GetLoginUrlAndClearCookie() const;

	UFUNCTION(BlueprintCallable)
	bool SendExchangeCodeForTokensRequest(FText changedUrl);

	// Server
	UFUNCTION()
	FString GetAuthorization() const;
};
