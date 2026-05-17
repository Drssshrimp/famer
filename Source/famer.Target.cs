using UnrealBuildTool;
using System.Collections.Generic;

public class famerTarget : TargetRules
{
	public famerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CatchAnimals");
	}
}
