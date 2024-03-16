// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class project_two : ModuleRules
{
	public project_two(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// 기본
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { });

        // 추가
        PublicDependencyModuleNames.AddRange(new string[] { "PhysicsCore", "Chaos", "Networking", "Sockets", "NetCore" });
        PublicDependencyModuleNames.AddRange(new string[] { "UMG", "HeadMountedDisplay" });
        PublicDependencyModuleNames.AddRange(new string[] { "SlateCore", "WebBrowserWidget", });
        PublicDependencyModuleNames.AddRange(new string[] { "LevelSequence", "MovieScene", });
        PublicDependencyModuleNames.AddRange(new string[] { "AWSConnector", "ServerManagerConnector", "WhaleTekAWS" });

        //PrivateDependencyModuleNames.AddRange(new string[] { "VivoxCore" });

        // include
        //PublicIncludePaths.AddRange(new string[] { "../Plugins/ServerManagerConnector/Source/ServerManagerConnector/Public"});

    }
}
