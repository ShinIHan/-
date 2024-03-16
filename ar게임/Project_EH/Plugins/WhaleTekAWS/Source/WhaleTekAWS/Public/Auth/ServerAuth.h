// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Auth.h"
#include "ServerAuth.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WHALETEKAWS_API UServerAuth : public UAuth
{
	GENERATED_BODY()
	
public:
	UServerAuth();
	virtual ~UServerAuth() override;

	virtual void Initialize() override;
	virtual void Start(UAWSSubsystem* pSubsystem) override;
	virtual void Shutdown() override;
};
