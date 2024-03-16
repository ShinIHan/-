// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Auth/Auth.h"
#include "APIGateway/APIGateway.h"
#include "APIGateway/DynamoDBAPIGateway.h"
#include "APIGateway/EC2APIGateway.h"
#include "APIGateway/CognitoAPIGateway.h"
#include "AWSSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WHALETEKAWS_API UAWSSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
		UAuth* pAuthorizer;

	UPROPERTY()
		UDynamoDBAPIGateway* pDynamoDBGateway;

	UPROPERTY()
		UCognitoAPIGateway* pCognitoGateway;

	UPROPERTY()
		UEC2APIGateway* pEC2Gateway;

public:
	UAWSSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
		bool StartAuthorizer();

	UFUNCTION(BlueprintCallable)
		bool StartGateways();

	UFUNCTION()
		bool StartDynamoDBGateway();

	UFUNCTION()
		bool StartCognitoGateway();

	UFUNCTION()
		bool StartEC2Gateway();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UAuth* GetAuthorizer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UDynamoDBAPIGateway* GetDynamoDBGateway();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UCognitoAPIGateway* GetCognitoGateway();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UEC2APIGateway* GetEC2Gateway();
};
