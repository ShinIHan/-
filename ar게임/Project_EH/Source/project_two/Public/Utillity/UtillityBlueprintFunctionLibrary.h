// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtillityBlueprintFunctionLibrary.generated.h"

/**
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.17
 * 레퍼런스 : VRExpansionPlugin, Project_One
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
