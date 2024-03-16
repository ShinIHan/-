// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Auth.generated.h"

class UAWSSubsystem;
/**
 * 
 */
UCLASS(Blueprintable)
class WHALETEKAWS_API UAuth : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
		UAWSSubsystem* pAWSSubsystem;

	UPROPERTY()
		FString Token;

public:
	UAuth();
	virtual ~UAuth() override;

	virtual void Initialize();
	virtual void Start(UAWSSubsystem* pSubsystem);
	virtual void Shutdown();

	UFUNCTION()
	FString GetAuthorization() const;	
};
