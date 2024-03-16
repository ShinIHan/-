// Fill out your copyright notice in the Description page of Project Settings.


#include "APIGateway/APIGateway.h"

UAPIGateway::UAPIGateway()
{
	Initialize();
}

UAPIGateway::~UAPIGateway()
{

}

void UAPIGateway::Initialize()
{
	pHttpModule = &FHttpModule::Get();
}

void UAPIGateway::Start(UAWSSubsystem* pSubsystem)
{
	pAWSSubsystem = pSubsystem;
}

void UAPIGateway::Shutdown()
{

}