// Copyright (c) Bruno Pereira Costa

using UnrealBuildTool;
using System.Collections.Generic;

public class SpheriactusEditorTarget : TargetRules
{
	public SpheriactusEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "Spheriactus" } );
	}
}