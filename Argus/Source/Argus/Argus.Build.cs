// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Argus : ModuleRules
{
	public Argus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "UnrealEd", "Engine", "InputCore", "EnhancedInput", "NavigationSystem" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PrivateIncludePaths.AddRange(new string[] { "Argus/ECS/", "Argus/Utilities/", "Argus/UnrealObjects/", "Argus/FunctionalTesting/", "Argus/StaticData/" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
