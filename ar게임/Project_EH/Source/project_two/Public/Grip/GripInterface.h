// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Grip/GripDataType.h"

#include "UObject/Interface.h"
#include "GripInterface.generated.h"

class UGripComponent;

// This class does not need to be modified.
// 수정할 필요는 없으나 언리얼 인터페이스 구현 구조상 존재해야 합니다.
UINTERFACE(MinimalAPI, Blueprintable)
class UGripInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()

};

/**
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.22
 * 레퍼런스 : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */
class PROJECT_TWO_API IGripInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	EGripType GetGripType();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	EGripRepMoveType GetGripRepMoveType();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface", meta = (DisplayName = "IsDenyingGrips"))
	bool bDenyGrip(UGripComponent* GripComponent = nullptr);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface", meta = (DisplayName = "DropSimulate"))
	bool bDropSimulate();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface", meta = (DisplayName = "IsDenyHighlight"))
	bool bDenyHighlight(UGripComponent* GripComponent = nullptr);

	//*********Main Implement*************//
	//오브젝트를 집고 있는 동안 매 틱마다 실행, 커스텀한 움직임이나 그립시 로직 구현 가능
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void TickGrip(UGripComponent* GripComponent, const FGripInformation& GripInformation, float DeltaTime);

	//인터페이스 상속받은 객체를 집었을 때 실행됨.
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnGrip(UGripComponent* GripComponent, const FGripInformation& GripInformation);
	
	//인터페이스 상속받은 객체를 집고 있다 놓을 때 실행됨.
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnGripRelease(UGripComponent* ReleasingGripComponent, const FGripInformation& GripInformation);

	//감지되면 호출
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnDetect(UGripComponent* GripComponent);
	
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnEndDetect(UGripComponent* ReleasingGripComponent);

	//객체를 집고 있는 컴포넌트들이 있는지 반환.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void IsHeld(TArray<UGripComponent*>& HoldingComponents, bool& isHeld);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void SetHeld(UGripComponent* HoldingComponent, bool bIsHeld);

	/****** Interaction Implements *****/
	// 오브젝트 Trigger 호출. Ex.Gun
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnTriggerPressed();

	// 오브젝트 Trigger 끝날때 실행됨.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnTriggerReleased();

	//조이스틱 입력
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnAxisX(float AxisX);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnAxisY(float AxisY);
};
