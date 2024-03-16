// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WidgetType.generated.h"

/*
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.05.02.
 * 레퍼런스 : VRExpansionPlugin, Project_One
 *
*/

USTRUCT(BlueprintType, Category = "WidgetType")
struct PROJECT_TWO_API FDisplayWidgetData
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DisplayWidget|Customize")
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DisplayWidget|Customize")
	FString Description;
};