// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "interaction/InteractionInterface.h"
#include "InteractiveWidgetComponent.generated.h"

/*
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.05.02.
 * 레퍼런스 : VRExpansionPlugin, Project_One
 *
 *  * Project One에 구현되어있던 Gaze 기능 배제
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
