// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DynamoDBSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AWSCONNECTOR_API UDynamoDBSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FString Authorization;

	UPROPERTY()
	FString ApiUrl;

	FHttpModule* pHttpModule;

private:
	void OnGetPlayerDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetPlayerCustomizeDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnModifyPlayerCustomizeDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnGetLectureDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetTimeTableDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnModifyAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnModifyPlayerSpawnableResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnModifyPlayerAccessibleResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnModifyPlayerInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	UDynamoDBSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	bool SendGetPlayerDataRequest(FString UserId = "");

	UFUNCTION(BlueprintCallable)
	bool SendGetPlayerCustomizeDataRequest(FString UserId = "");

	UFUNCTION(BlueprintCallable)
	bool SendModifyPlayerCustomizeDataRequest(FString UserId, TArray<int> CustomizeItems);
	
	UFUNCTION(BlueprintCallable)
	bool SendGetLectureDataRequest(FString LectureCode);

	UFUNCTION(BlueprintCallable)
	bool SendGetTimeTableDataRequest(FDateTime date, FString Classroom);

	UFUNCTION(BlueprintCallable)
	bool SendGetAttendanceRecordRequest(FString StudentSub);

	UFUNCTION(BlueprintCallable)
	bool SendModifyAttendanceRecordRequest(FString StudentSub, FString LectureStartedTime, FString Content);

	UFUNCTION(BlueprintCallable)
	bool SendModifyPlayerSpawnableRequest(FString UserId, TArray<FString> AddSpawnables, TArray<FString> DeleteSpawnables);

	UFUNCTION(BlueprintCallable)
	bool SendModifyPlayerAccessibleRequest(FString UserId, TArray<FString> AddAccessibles, TArray<FString> DeleteAccessibles);

	UFUNCTION(BlueprintCallable)
	bool SendModifyPlayerInventoryRequest(FString UserId, TArray<FString> AddItems, TArray<FString> DeleteItems);
	
};
