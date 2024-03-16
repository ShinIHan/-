// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DynamoDBCallbackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDynamoDBCallbackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AWSCONNECTOR_API IDynamoDBCallbackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Get Player Data ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedPlayerIdEvent(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedPlayerNameEvent(const FString& PlayerSub, const FString& PlayerName);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedPlayerRoleEvent(const FString& PlayerSub, const FString& PlayerRole);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedPlayerInfoEvent(const FString& PlayerSub, const TArray<FString>& Spawnables, const TArray<FString>& Accessibles, const TArray<FString>& Inventory);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetPlayerDataFailureEvent(const FString& FailureReason);
	
	// Get Player Customize Data ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetPlayerCustomizeDataSuccessEvent(const FString& PlayerSub, const TArray<int>& CustomizeItems);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetPlayerCustomizeDataFailureEvent(const FString& FailureReason);

	// Modify Player Customize Data ///////////////////////////////
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedModifyPlayerCustomizeDataSuccessEvent();
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedModifyPlayerCustomizeDataFailureEvent();

	// Get Lecture Data ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetLectureDataSuccessEvent(const FString& LectureCode, const FString& SchoolName, const FString& DepartmentName, const FString& LectureName, const TArray<FString>& UsableContents, const FString& ProfessorId, const TArray<FString>& StudentIds);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetLectureDataFailureEvent(const FString& FailureReason);

	// Get Time Table Data ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetTimeTableDataSuccessEvent(const FDateTime& StartTime, const FString& Classroom, const FString& Content, const TArray<FString>& Students);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetTimeTableDataFailureEvent(const FString& FailureReason);
	
	// Get Attendance Record  ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetAttendanceRecordSuccessEvent(const FString& playerSub, const FString& playerId, 
		const TArray<FString>& pt, const TArray<FString>& ps, 
		const TArray<FString>& em1, const TArray<FString>& em2, const TArray<FString>& em3,
		const TArray<FString>& sc1, const TArray<FString>& sc2, const TArray<FString>& sc3,
		const TArray<FString>& ec1, const TArray<FString>& ec2, const TArray<FString>& ec3);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedGetAttendanceRecordFailureEvent(const FString& FailureReason);

	// Modify Attendance Record  ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModifyAttendanceRecordSuccessEvent(const FString& studentSub);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModifyAttendanceRecordFailureEvent(const FString& FailureReason);

	// Modify Player Spawnable ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ReceivedModfiyPlayerSpawnableSuccessEvent(const FString& UserSub, const TArray<FString>& Spawnables);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ReceivedModfiyPlayerSpawnableFailureEvent(const FString& FailureReason);

	// Modify Player Accessible  ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModfiyPlayerAccessibleSuccessEvent(const FString& UserSub, const TArray<FString>& Accessibles);
    
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModfiyPlayerAccessibleFailureEvent(const FString& FailureReason);
	
	// Modify Player Inventory  ///////////////////////////////
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModfiyPlayerInventorySuccessEvent(const FString& UserSub, const TArray<FString>& Inventory);
    
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedModfiyPlayerInventoryFailureEvent(const FString& FailureReason);
};
