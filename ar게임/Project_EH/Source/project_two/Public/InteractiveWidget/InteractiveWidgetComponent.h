// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "interaction/InteractionInterface.h"
#include "InteractiveWidgetComponent.generated.h"

/*
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.05.02.
 * ���۷��� : VRExpansionPlugin, Project_One
 *
 *  * Project One�� �����Ǿ��ִ� Gaze ��� ����
*/

UCLASS(Blueprintable, ClassGroup = "InteractiveWidget", editinlinenew, meta = (BlueprintSpawnableComponent))
class UInteractiveWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	APlayerCameraManager* PlayerCameraManager = nullptr;

public:
	//TO ACTOR
	UPROPERTY(EditDefaultsOnly, Category = InteractiveWidget)
	bool bEnableGripMovement = false;
};
