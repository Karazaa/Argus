// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Argus : ModuleRules
{
	public Argus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "EnhancedInput", "ImGui", "InputCore", "NavigationSystem", "Navmesh", "UMG" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Editor only modules
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        PrivateIncludePaths.AddRange(new string[] { "Argus/ECS/", "Argus/Utilities/", "Argus/UnrealObjects/", "Argus/FunctionalTesting/", "Argus/StaticData/", "Argus/UI/", "Argus/Memory/"});

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
