// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Authorizer.h"
#include "ServerVerifier.generated.h"

/**
 * 
 */
UCLASS()
class AWSCONNECTOR_API UServerVerifier : public UAuthorizer
{
	GENERATED_BODY()
private:
	UPROPERTY()
	FString	ServerPassword;

public:
	UServerVerifier();
	virtual ~UServerVerifier() override;

	virtual void Initialize() override;
	virtual void Start(UGameInstance* gameInstance) override;
	virtual void Shutdown() override;

	FString GetServerPassword() const;
};
