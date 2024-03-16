// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtillityBlueprintFunctionLibrary.generated.h"

/**
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.17
 * ���۷��� : VRExpansionPlugin, Project_One
 *
 */
UCLASS()
class PROJECT_TWO_API UUtillityBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "UtillityBlueprintFunctionLibrary|Debug", meta = (DisplayName = "DrawDebugAxes"))
	static void DrawDebugAxes(const FTransform& Transform, float ArrowLength = 15, bool IsDrawSphere = false, float SphereRadius = 12, const FColor& SphereColor = FColor::Black);

};
