// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AWSBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AWSCONNECTOR_API UAWSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FString ReadFile(FString filePath);

	static void SeparateCustomizeCode(FString customizeCode, FString& itemCode, int& colorR, int& colorG, int& colorB);

	static FString ConvertColorNumberToColorCode(int colorR, int colorG, int colorB);
};
