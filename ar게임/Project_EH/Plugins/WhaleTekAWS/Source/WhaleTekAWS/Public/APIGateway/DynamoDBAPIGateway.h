// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "APIGateway/APIGateway.h"
#include "DynamoDBAPIGateway.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerSub;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString Role;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<int> CustomizeItems;
};

USTRUCT(BlueprintType)
struct FTimeTable
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString StartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString InstanceName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString ContentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString LevelName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int MaxPeople;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int RunningTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FString> Students;
};

USTRUCT(BlueprintType)
struct FAttendanceRecord
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerSub;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString LevelName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString ContentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FString> AttendanceList;
};

///////////////////////
// GetProjectVersion
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetProjectVersionSuccessEvent, FString, ProjectVersion);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetProjectVersionFailureEvent, FString, FailureReason);

///////////////////////
// GetMyInfo
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyInfoSuccessEvent, FPlayerInfo, PlayerInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyInfoFailureEvent, FString, FailureReason);

// ModifyMyCustomize
DECLARE_DYNAMIC_DELEGATE_OneParam(FModifyMyCustomizeSuccessEvent, const TArray<int>&, CustomizeItems);
DECLARE_DYNAMIC_DELEGATE_OneParam(FModifyMyCustomizeFailureEvent, FString, FailureReason);

// GetPlayerInfo
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetPlayerInfoSuccessEvent, FPlayerInfo, PlayerInfo);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetPlayerInfoFailureEvent, FString, FailureReason, FString, PlayerSub);

///////////////////////
// GetTimeTable
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetTimeTableSuccessEvent, const TArray<FTimeTable>&, Classes);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetTimeTableFailureEvent, FString, FailureReason);

// AddToTimeTable_Kiosk
DECLARE_DYNAMIC_DELEGATE_OneParam(FAddToTimeTableKioskSuccessEvent, FTimeTable, TimeTable);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FAddToTimeTableKioskFailureEvent, FString, FailureReason, FTimeTable, TimeTable);

// SignUpClass_Kiosk
DECLARE_DYNAMIC_DELEGATE_OneParam(FSignUpClassKioskSuccessEvent, bool, SignUpStatus);
DECLARE_DYNAMIC_DELEGATE_OneParam(FSignUpClassKioskFailureEvent, FString, FailureReason);

// CheckSignUpClass_Kiosk
DECLARE_DYNAMIC_DELEGATE_OneParam(FCheckSignUpClassKioskSuccessEvent, bool, CheckSignUpStatus);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCheckSignUpClassKioskFailureEvent, FString, FailureReason);

// GetMyClass_Kiosk
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyClassKioskSuccessEvent, const TArray<FTimeTable>&, Classes);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyClassKioskFailureEvent, FString, FailureReason);

///////////////////////
// GetMyAttendanceRecord
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyAttendanceRecordSuccessEvent, const TArray<FAttendanceRecord>&, AttendanceRecords);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetMyAttendanceRecordFailureEvent, FString, FailureReason);

// AddAttendanceRecord
DECLARE_DYNAMIC_DELEGATE_OneParam(FAddAttendanceRecordSuccessEvent, FAttendanceRecord, AttendanceRecord);
DECLARE_DYNAMIC_DELEGATE_OneParam(FAddAttendanceRecordFailureEvent, FString, FailureReason);

///////////////////////
// GetVlcURL
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetVlcURLSuccessEvent, FString, VlcURL, bool, IsLive);
DECLARE_DYNAMIC_DELEGATE_OneParam(FGetVlcURLFailureEvent, FString, FailureReason);

///////////////////////
// AddAccessRecord
DECLARE_DYNAMIC_DELEGATE(FAddAccessRecordSuccessEvent);
DECLARE_DYNAMIC_DELEGATE_OneParam(FAddAccessRecordFailureEvent, FString, FailureReason);

UCLASS(Blueprintable)
class WHALETEKAWS_API UDynamoDBAPIGateway : public UAPIGateway
{
	GENERATED_BODY()

private:
	///////////////////////
	void OnGetProjectVersionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetProjectVersionSuccessEvent SuccessDelegate, FGetProjectVersionFailureEvent FailureDelegate);

	///////////////////////
	void OnGetMyInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyInfoSuccessEvent SuccessDelegate, FGetMyInfoFailureEvent FailureDelegate);
	void OnModifyMyCustomizeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FModifyMyCustomizeSuccessEvent SuccessDelegate, FModifyMyCustomizeFailureEvent FailureDelegate);
	void OnGetPlayerInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetPlayerInfoSuccessEvent SuccessDelegate, FGetPlayerInfoFailureEvent FailureDelegate);
	
	///////////////////////
	void OnGetTimeTableResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetTimeTableSuccessEvent SuccessDelegate, FGetTimeTableFailureEvent FailureDelegate);
	void OnAddToTimeTableKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddToTimeTableKioskSuccessEvent SuccessDelegate, FAddToTimeTableKioskFailureEvent FailureDelegate);
	void OnSignUpClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FSignUpClassKioskSuccessEvent SuccessDelegate, FSignUpClassKioskFailureEvent FailureDelegate);
	void OnCheckSignUpClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FCheckSignUpClassKioskSuccessEvent SuccessDelegate, FCheckSignUpClassKioskFailureEvent FailureDelegate);
	void OnGetMyClassKioskResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyClassKioskSuccessEvent SuccessDelegate, FGetMyClassKioskFailureEvent FailureDelegate);
	
	///////////////////////
	void OnGetMyAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetMyAttendanceRecordSuccessEvent SuccessDelegate, FGetMyAttendanceRecordFailureEvent FailureDelegate);
	void OnAddAttendanceRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddAttendanceRecordSuccessEvent SuccessDelegate, FAddAttendanceRecordFailureEvent FailureDelegate);

	///////////////////////
	void OnGetVlcURLResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FGetVlcURLSuccessEvent SuccessDelegate, FGetVlcURLFailureEvent FailureDelegate);

	///////////////////////
	void OnAddAccessRecordResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FAddAccessRecordSuccessEvent SuccessDelegate, FAddAccessRecordFailureEvent FailureDelegate);
public:
	UDynamoDBAPIGateway();
	virtual ~UDynamoDBAPIGateway() override;

	virtual void Initialize() override;
	virtual void Start(UAWSSubsystem* pSubsystem) override;
	virtual void Shutdown() override;

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Client_ProjectVersion")
		bool SendGetProjectVersionRequest(FGetProjectVersionSuccessEvent SuccessDelegate, FGetProjectVersionFailureEvent FailureDelegate);

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Client_Info")
		bool SendGetMyInfoRequest(FGetMyInfoSuccessEvent SuccessDelegate, FGetMyInfoFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Client_Info")
		bool SendModifyMyCustomizeRequest(TArray<int> CustomizeItems, FModifyMyCustomizeSuccessEvent SuccessDelegate, FModifyMyCustomizeFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Server_Info")
		bool SendGetPlayerInfoRequest(FString PlayerSub, FGetPlayerInfoSuccessEvent SuccessDelegate, FGetPlayerInfoFailureEvent FailureDelegate);

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Server_TimeTable")
		bool SendGetTimeTableRequest(FString StartTime, FString InstanceName, FGetTimeTableSuccessEvent SuccessDelegate, FGetTimeTableFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Server_TimeTable")
		bool SendAddToTimeTableKioskRequest(FString StartTime, FString LevelName, FAddToTimeTableKioskSuccessEvent SuccessDelegate, FAddToTimeTableKioskFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Client_TimeTable")
		bool SendSignUpClassKioskRequest(FString StartTime, FString LevelName, bool SignUp, FSignUpClassKioskSuccessEvent SuccessDelegate, FSignUpClassKioskFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Client_TimeTable")
		bool SendCheckSignUpClassKioskRequest(FString StartTime, FString InstanceName, FCheckSignUpClassKioskSuccessEvent SuccessDelegate, FCheckSignUpClassKioskFailureEvent FailureDelegate);
	
	UFUNCTION(BlueprintCallable, Category = "Client_TimeTable")
		bool SendGetMyClassKioskRequest(FString StartTime, FGetMyClassKioskSuccessEvent SuccessDelegate, FGetMyClassKioskFailureEvent FailureDelegate);

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Client_AttendanceRecord")
		bool SendGetMyAttendanceRecordRequest(FGetMyAttendanceRecordSuccessEvent SuccessDelegate, FGetMyAttendanceRecordFailureEvent FailureDelegate);

	UFUNCTION(BlueprintCallable, Category = "Server_AttendanceRecord")
		bool SendAddAttendanceRecordRequest(FString PlayerSub, FString LevelName, FString AttendanceDate, FAddAttendanceRecordSuccessEvent SuccessDelegate, FAddAttendanceRecordFailureEvent FailureDelegate);

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Server_VlcURL")
		bool SendGetVlcURLRequest(FGetVlcURLSuccessEvent SuccessDelegate, FGetVlcURLFailureEvent FailureDelegate);

	///////////////////////
	UFUNCTION(BlueprintCallable, Category = "Server_AddAccessRecord")
		bool SendAddAccessRecordRequest(FString PlayerSub, FString LevelName, FString AccessTime, FAddAccessRecordSuccessEvent SuccessDelegate, FAddAccessRecordFailureEvent FailureDelegate);
};
