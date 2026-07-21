// Copyright Karazaa. This is a part of an RTS project called Argus.

using EpicGames.Core;
using UnrealBuildTool;

public class ArgusEditor : ModuleRules
{
	public ArgusEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.Add("ArgusEditor");

		PublicDependencyModuleNames.AddAll(
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
           		"Slate",
            		"SlateCore",
            		"PropertyEditor",
            		"Argus"
		);

		PrivateDependencyModuleNames.AddAll();
	}
}