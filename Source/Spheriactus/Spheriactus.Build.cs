// Copyright (c) Bruno Pereira Costa

using UnrealBuildTool;

public class Spheriactus : ModuleRules
{
	public Spheriactus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "LidarPointCloudRuntime", "VRExpansionPlugin" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "AppFramework" });
	}
}