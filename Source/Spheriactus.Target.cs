// Copyright (c) Bruno Pereira Costa

using UnrealBuildTool;
using System.Collections.Generic;

public class SpheriactusTarget : TargetRules
{
	public SpheriactusTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "Spheriactus" } );
	}
}