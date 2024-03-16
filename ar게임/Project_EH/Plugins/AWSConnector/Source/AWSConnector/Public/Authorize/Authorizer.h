// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "UObject/NoExportTypes.h"
#include "Authorizer.generated.h"

/**
 * 
 */
UCLASS()
class AWSCONNECTOR_API UAuthorizer : public UObject
{
	GENERATED_BODY()

private:
	FHttpModule* pHttpModule;
	
	UPROPERTY()
	FString ApiUrl;
	
protected:
	UPROPERTY()
	UGameInstance* pGameInstance;
	
public:
	UAuthorizer();
	virtual ~UAuthorizer() override;

	virtual void Initialize();
	virtual void Start(UGameInstance* GameInstance);
	virtual void Shutdown();

	FString GetApiUrl() const;
	FHttpModule* GetHttpModule(); 
};
