// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WidgetType.generated.h"

/*
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.05.02.
 * ���۷��� : VRExpansionPlugin, Project_One
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