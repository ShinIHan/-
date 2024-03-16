// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AWSFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class WHALETEKAWS_API UAWSFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FString ReadFile(FString FilePath);
};
