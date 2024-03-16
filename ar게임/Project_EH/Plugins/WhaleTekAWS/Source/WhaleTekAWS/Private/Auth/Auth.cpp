// Fill out your copyright notice in the Description page of Project Settings.


#include "Auth/Auth.h"

UAuth::UAuth()
{
	
}

UAuth::~UAuth()
{

}

void UAuth::Initialize()
{

}

void UAuth::Start(UAWSSubsystem* pSubsystem)
{
	pAWSSubsystem = pSubsystem;
}

void UAuth::Shutdown()
{

}

FString UAuth::GetAuthorization() const
{
	return Token;
}