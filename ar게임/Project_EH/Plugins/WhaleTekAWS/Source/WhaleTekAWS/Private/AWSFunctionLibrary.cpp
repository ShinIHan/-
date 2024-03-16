// Fill out your copyright notice in the Description page of Project Settings.


#include "AWSFunctionLibrary.h"

FString UAWSFunctionLibrary::ReadFile(FString FilePath)
{
	FString DirectoryPath = FPaths::ProjectContentDir();
	FString FullPath = DirectoryPath + "/" + FilePath;
	FString Result;
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();

	if (File.FileExists(*FullPath)) {
		FFileHelper::LoadFileToString(Result, *FullPath);
	}

	return Result;
}