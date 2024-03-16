// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "APIGateway.h"
#include "EC2APIGateway.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FTag
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString Key;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString Value;
};

USTRUCT(BlueprintType)
struct FEC2Info
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString InstanceId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString PublicAddress;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString State;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TArray<FTag> Tags;
};

//GetEC2State_TagName
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetEC2StateTagNameSuccessEvent, const TArray<FEC2Info>&, InstancesInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetEC2StateTagNameFailureEvent, FString, FailureReason);

UCLASS(Blueprintable)
class WHALETEKAWS_API UEC2APIGateway : public UAPIGateway
{
	GENERATED_BODY()

private:
	///////////////////////
	void OnGetEC2StateTagNameResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetEC2StateTagNameSuccessEvent SuccessDelegate, FGetEC2StateTagNameFailureEvent FailureDelegate);
public:
	UEC2APIGateway();
	virtual ~UEC2APIGateway() override;


	virtual void Initialize() override;
	virtual void Start(UAWSSubsystem* pSubsystem) override;
	virtual void Shutdown() override;


	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "EC2")
		bool SendGetEC2StateTagNameRequest(FString TagName, FGetEC2StateTagNameSuccessEvent SuccessDelegate, FGetEC2StateTagNameFailureEvent FailureDelegate);
};
