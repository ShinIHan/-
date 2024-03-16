// Fill out your copyright notice in the Description page of Project Settings.


#include "AWSBlueprintFunctionLibrary.h"
#include <string>

FString UAWSBlueprintFunctionLibrary::ReadFile(FString filePath)
{
	FString directoryPath = FPaths::ProjectContentDir();
	FString fullPath = directoryPath + "/" + filePath;
	FString result;
	IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();

	if (file.FileExists(*fullPath)) {
		FFileHelper::LoadFileToString(result, *fullPath);
	}

	return result;
}

void UAWSBlueprintFunctionLibrary::SeparateCustomizeCode(FString customizeCode, FString& itemCode, int& colorR,
	int& colorG, int& colorB)
{
	std::string customizeCodeStr = TCHAR_TO_ANSI(*customizeCode);

	itemCode = *FString((customizeCodeStr.substr(0, 4)).c_str());
	colorR = atoi((customizeCodeStr.substr(4, 3)).c_str());
	colorG = atoi((customizeCodeStr.substr(7, 3)).c_str());
	colorB = atoi((customizeCodeStr.substr(10, 3)).c_str());
}

FString UAWSBlueprintFunctionLibrary::ConvertColorNumberToColorCode(int colorR, int colorG, int colorB)
{
	FString redCode = FString::FromInt(colorR);
	while (redCode.Len() < 3) redCode = "0" + redCode;

	FString greenCode = FString::FromInt(colorG);
	while (greenCode.Len() < 3) greenCode = "0" + greenCode;

	FString blueCode = FString::FromInt(colorB);
	while (blueCode.Len() < 3) blueCode = "0" + blueCode;

	return redCode + greenCode + blueCode;
}
