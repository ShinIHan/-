// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NetworkCallbackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UNetworkCallbackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SERVERMANAGERCONNECTOR_API INetworkCallbackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedVivoxChannelNameEvent(const FString& channelName);

	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedInviteEvent(const FDateTime& lectureStartTime, 
	//	//const int& startYear, const int& startMonth, const int& startDay, const int& startHour, const int& startMinute,
	//	const FString& classroom, const FString& lectureCode, const FString& content);
	//
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedLectureEvent(const FDateTime& lectureStartTime, const FDateTime& lectureEndTime,
	//	//const int& endYear, const int& endMonth, const int& endDay, const int& endHour, const int& endMinute,
	//	const FString& classroom, const FString& lectureCode, const FString& content);
	//
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedEventResponseEvent(const int& eventType);
	
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedMatchmakingSuccessEvent(const FString& levelName);
	//
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//void ReceivedMatchmakingFailureEvent();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceivedDisconnectPlayerRequestEvent(const FString& playerSub);
};
