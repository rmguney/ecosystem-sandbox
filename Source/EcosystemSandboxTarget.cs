using UnrealBuildTool;
using System.Collections.Generic;

public class EcosystemSandboxTarget : TargetRules
{
	public EcosystemSandboxTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "EcosystemSandbox" });
		
		// Enable Chaos Physics
		bUseChaos = true;
		
		// Optimization settings
		bUseUnityBuild = true;
		bUsePCHFiles = true;
	}
}
