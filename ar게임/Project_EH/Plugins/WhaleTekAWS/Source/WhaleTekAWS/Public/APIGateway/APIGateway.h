// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "APIGateway.generated.h"

class UAWSSubsystem;

UCLASS()
class WHALETEKAWS_API UAPIGateway : public UObject
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
		UAWSSubsystem* pAWSSubsystem;

	UPROPERTY()
		FString ApiUrl;

	FHttpModule* pHttpModule;

public:
	UAPIGateway();
	virtual ~UAPIGateway() override;

	virtual void Initialize();
	virtual void Start(UAWSSubsystem* pSubsystem);
	virtual void Shutdown();
};
