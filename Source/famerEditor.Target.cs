using UnrealBuildTool;
using System.Collections.Generic;

public class famerEditorTarget : TargetRules
{
	public famerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CatchAnimals");
	}
}
