// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Grip/GripDataType.h"

#include "UObject/Interface.h"
#include "GripInterface.generated.h"

class UGripComponent;

// This class does not need to be modified.
// ������ �ʿ�� ������ �𸮾� �������̽� ���� ������ �����ؾ� �մϴ�.
UINTERFACE(MinimalAPI, Blueprintable)
class UGripInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()

};

/**
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.22
 * ���۷��� : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
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
	//������Ʈ�� ���� �ִ� ���� �� ƽ���� ����, Ŀ������ �������̳� �׸��� ���� ���� ����
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void TickGrip(UGripComponent* GripComponent, const FGripInformation& GripInformation, float DeltaTime);

	//�������̽� ��ӹ��� ��ü�� ������ �� �����.
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnGrip(UGripComponent* GripComponent, const FGripInformation& GripInformation);
	
	//�������̽� ��ӹ��� ��ü�� ���� �ִ� ���� �� �����.
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnGripRelease(UGripComponent* ReleasingGripComponent, const FGripInformation& GripInformation);

	//�����Ǹ� ȣ��
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnDetect(UGripComponent* GripComponent);
	
	UFUNCTION(BlueprintNativeEvent, Category = "GripInterface")
	void OnEndDetect(UGripComponent* ReleasingGripComponent);

	//��ü�� ���� �ִ� ������Ʈ���� �ִ��� ��ȯ.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void IsHeld(TArray<UGripComponent*>& HoldingComponents, bool& isHeld);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void SetHeld(UGripComponent* HoldingComponent, bool bIsHeld);

	/****** Interaction Implements *****/
	// ������Ʈ Trigger ȣ��. Ex.Gun
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnTriggerPressed();

	// ������Ʈ Trigger ������ �����.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnTriggerReleased();

	//���̽�ƽ �Է�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnAxisX(float AxisX);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GripInterface")
	void OnAxisY(float AxisY);
};
