using UnrealBuildTool;

public class EcosystemSandbox : ModuleRules
{
	public EcosystemSandbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"AIModule",
			"GameplayTags",
			"NavigationSystem",
			"ChaosSolverEngine",
			"GeometryCollectionEngine",
			"Chaos",
			"PhysicsCore",
			"Niagara",
			"NiagaraCore",
			"Renderer",
			"RenderCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate",
			"SlateCore",
			"ToolMenus",
			"EditorStyle",
			"EditorWidgets",
			"UnrealEd",
			"LevelEditor"
		});
		
		// Enable Nanite support
		PublicDefinitions.Add("WITH_NANITE=1");
		
		// Enable Chaos Physics
		PublicDefinitions.Add("WITH_CHAOS=1");
	}
}
