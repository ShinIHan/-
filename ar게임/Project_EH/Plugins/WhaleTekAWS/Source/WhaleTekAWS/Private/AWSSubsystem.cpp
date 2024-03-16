// Fill out your copyright notice in the Description page of Project Settings.


#include "AWSSubsystem.h"
#include "Auth/ServerAuth.h"
#include "Auth/CognitoAuth.h"

UAWSSubsystem::UAWSSubsystem()
{

}

void UAWSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (true == GetGameInstance()->IsDedicatedServerInstance()) {
		pAuthorizer = NewObject<UServerAuth>(GetTransientPackage(), UServerAuth::StaticClass());
	}
	else {
		pAuthorizer = NewObject<UCognitoAuth>(GetTransientPackage(), UCognitoAuth::StaticClass());
	}
	pAuthorizer->AddToRoot();

	pDynamoDBGateway = NewObject<UDynamoDBAPIGateway>(GetTransientPackage(), UDynamoDBAPIGateway::StaticClass());
	pDynamoDBGateway->AddToRoot();

	pCognitoGateway = NewObject<UCognitoAPIGateway>(GetTransientPackage(), UCognitoAPIGateway::StaticClass());
	pCognitoGateway->AddToRoot();

	pEC2Gateway = NewObject<UEC2APIGateway>(GetTransientPackage(), UEC2APIGateway::StaticClass());
	pEC2Gateway->AddToRoot();
}

void UAWSSubsystem::Deinitialize()
{
	pEC2Gateway->RemoveFromRoot();
	pCognitoGateway->RemoveFromRoot();
	pDynamoDBGateway->RemoveFromRoot();
	pAuthorizer->RemoveFromRoot();
	Super::Deinitialize();
}

bool UAWSSubsystem::StartAuthorizer()
{
	if (pAuthorizer != nullptr) {
		pAuthorizer->Start(this);
		UE_LOG(LogTemp, Log, TEXT("UAWSSubsystem::StartAuthorizer() Complete - Bind Subsystem into Authorizer"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UAWSSubsystem::StartAuthorizer() Error - Authorizer is nullptr"));
		return false;
	}
}

bool UAWSSubsystem::StartGateways()
{
	if (StartCognitoGateway() == false) return false;
	if (StartEC2Gateway() == false) return false;
	return StartDynamoDBGateway();
}

bool UAWSSubsystem::StartDynamoDBGateway()
{
	if (pDynamoDBGateway != nullptr) {
		pDynamoDBGateway->Start(this);
		UE_LOG(LogTemp, Log, TEXT("UAWSSubsystem::StartDynamoDBGateway() Complete - Bind Subsystem into DynamoDBGateway"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UAWSSubsystem::StartDynamoDBGateway() Error - DynamoDBGateway is nullptr"));
		return false;
	}
}

bool UAWSSubsystem::StartCognitoGateway()
{
	if (pCognitoGateway != nullptr) {
		pCognitoGateway->Start(this);
		UE_LOG(LogTemp, Log, TEXT("UAWSSubsystem::StartCognitoGateway() Complete - Bind Subsystem into CognitoGateway"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UAWSSubsystem::StartCognitoGateway() Error - CognitoGateway is nullptr"));
		return false;
	}
}

bool UAWSSubsystem::StartEC2Gateway()
{
	if (pEC2Gateway != nullptr) {
		pEC2Gateway->Start(this);
		UE_LOG(LogTemp, Log, TEXT("UAWSSubsystem::StartEC2Gateway() Complete - Bind Subsystem into EC2Gateway"));
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UAWSSubsystem::StartEC2Gateway() Error - EC2Gateway is nullptr"));
		return false;
	}
}

UAuth* UAWSSubsystem::GetAuthorizer()
{
	return pAuthorizer;
}

UDynamoDBAPIGateway* UAWSSubsystem::GetDynamoDBGateway()
{
	return pDynamoDBGateway;
}

UEC2APIGateway* UAWSSubsystem::GetEC2Gateway()
{
	return pEC2Gateway;
}

UCognitoAPIGateway* UAWSSubsystem::GetCognitoGateway()
{
	return pCognitoGateway;
}