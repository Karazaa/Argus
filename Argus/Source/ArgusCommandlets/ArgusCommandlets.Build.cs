// Copyright Karazaa. This is a part of an RTS project called Argus.

using EpicGames.Core;
using UnrealBuildTool;

public class ArgusCommandlets : ModuleRules
{
	public ArgusCommandlets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.Add("ArgusCommandlets");

		PublicDependencyModuleNames.AddAll(
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
            "Argus"
		);

		PrivateDependencyModuleNames.AddAll();
	}
}