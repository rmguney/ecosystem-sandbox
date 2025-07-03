using UnrealBuildTool;
using System.Collections.Generic;

public class EcosystemSandboxEditorTarget : TargetRules
{
	public EcosystemSandboxEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] { "EcosystemSandbox" });
		
		// Enable Chaos Physics
		bUseChaos = true;
		
		// Editor optimizations
		bUseUnityBuild = true;
		bUsePCHFiles = true;
	}
}
