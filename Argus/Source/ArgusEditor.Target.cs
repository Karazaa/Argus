// Copyright Epic Games, Inc. All Rights Reserved.

using EpicGames.Core;
using UnrealBuildTool;

public class ArgusEditorTarget : TargetRules
{
	public ArgusEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddAll("Argus", "ArgusEditor");

		bAdaptiveUnityDisablesOptimizations = true;
	}
}
