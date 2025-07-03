#include "NaniteEnvironment.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/BodySetup.h"

ANaniteEnvironment::ANaniteEnvironment()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f; // Update every second for performance

	// Create terrain component
	TerrainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;

	// Enable Nanite for terrain
	TerrainMesh->bEvaluateWorldPositionOffset = true;
}

void ANaniteEnvironment::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeEnvironment();
}

void ANaniteEnvironment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle regrowth
	if (bEnableRegrowth && GetWorld()->GetTimeSeconds() - LastRegrowthCheck > RegrowthCheckInterval)
	{
		RegenerateDestroyedFoliage();
		LastRegrowthCheck = GetWorld()->GetTimeSeconds();
	}
}

void ANaniteEnvironment::InitializeEnvironment()
{
	SetupNaniteComponents();
	SetupFoliageComponents();
	GenerateFoliage();

	UE_LOG(LogTemp, Warning, TEXT("NaniteEnvironment initialized with %d foliage instances"), FoliageInstances.Num());
}

void ANaniteEnvironment::SetupNaniteComponents()
{
	if (NaniteTerrainMesh && TerrainMesh)
	{
		TerrainMesh->SetStaticMesh(NaniteTerrainMesh);
		
		// Enable Nanite if available
		if (TerrainMesh->GetStaticMesh())
		{
			// Force Nanite settings
			TerrainMesh->SetForcedLodModel(0);
			TerrainMesh->bOverrideMinLOD = false;
		}

		// Set terrain scale
		FVector TerrainScale = FVector(EnvironmentSize / 1000.0f);
		TerrainMesh->SetWorldScale3D(TerrainScale);

		// Enable collision for physics interaction
		TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TerrainMesh->SetCollisionObjectType(ECC_WorldStatic);
	}
}

void ANaniteEnvironment::SetupFoliageComponents()
{
	// Create instanced mesh components for each foliage type
	for (int32 i = 0; i < FoliageConfigs.Num(); i++)
	{
		const FNaniteFoliageConfig& Config = FoliageConfigs[i];
		
		if (!Config.NaniteMesh)
		{
			continue;
		}

		// Create instanced static mesh component
		UInstancedStaticMeshComponent* InstancedComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
			*FString::Printf(TEXT("FoliageComponent_%d"), i)
		);
		
		if (InstancedComponent)
		{
			InstancedComponent->SetupAttachment(RootComponent);
			InstancedComponent->SetStaticMesh(Config.NaniteMesh);
			InstancedComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			InstancedComponent->SetCollisionObjectType(ECC_WorldStatic);
			InstancedComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

			// Enable Nanite optimization
			InstancedComponent->SetCullDistances(0, 50000); // Large cull distance for Nanite
			
			FoliageComponents.Add(Config.FoliageType, InstancedComponent);
			FoliageTypeIndices.Add(Config.FoliageType, i);
		}
	}
}

void ANaniteEnvironment::GenerateFoliage()
{
	FoliageInstances.Empty();

	for (int32 ConfigIndex = 0; ConfigIndex < FoliageConfigs.Num(); ConfigIndex++)
	{
		const FNaniteFoliageConfig& Config = FoliageConfigs[ConfigIndex];
		
		for (int32 i = 0; i < Config.InstanceCount; i++)
		{
			SpawnFoliageInstance(Config, ConfigIndex);
		}
	}
}

void ANaniteEnvironment::SpawnFoliageInstance(const FNaniteFoliageConfig& Config, int32 ConfigIndex)
{
	// Get random position
	FVector Position = GetRandomPositionInRadius(Config.SpawnRadius);
	
	if (!IsValidSpawnLocation(Position))
	{
		return;
	}

	// Create transform
	FTransform InstanceTransform;
	InstanceTransform.SetLocation(Position);
	InstanceTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
	
	// Random scale
	float Scale = FMath::RandRange(Config.ScaleRange.X, Config.ScaleRange.Y);
	InstanceTransform.SetScale3D(FVector(Scale));

	// Add to instanced component
	UInstancedStaticMeshComponent* InstancedComponent = FoliageComponents.FindRef(Config.FoliageType);
	if (InstancedComponent)
	{
		int32 InstanceIndex = InstancedComponent->AddInstance(InstanceTransform);

		// Create foliage data
		FFoliageInstanceData InstanceData;
		InstanceData.FoliageType = Config.FoliageType;
		InstanceData.Transform = InstanceTransform;
		InstanceData.Health = 100.0f;
		InstanceData.RegrowthTime = Config.FoliageType == EFoliageType::Tree ? 120.0f : 60.0f;

		FoliageInstances.Add(InstanceData);
	}
}

bool ANaniteEnvironment::DamageFoliage(const FVector& Location, float Radius, float Damage)
{
	bool bAnyDamage = false;

	for (int32 i = 0; i < FoliageInstances.Num(); i++)
	{
		FFoliageInstanceData& Instance = FoliageInstances[i];
		
		if (Instance.bIsDestroyed)
		{
			continue;
		}

		float Distance = FVector::Dist(Instance.Transform.GetLocation(), Location);
		if (Distance <= Radius)
		{
			// Apply damage with distance falloff
			float DamageMultiplier = 1.0f - (Distance / Radius);
			float ActualDamage = Damage * DamageMultiplier;
			
			Instance.Health -= ActualDamage;
			bAnyDamage = true;

			// Check if destroyed
			if (Instance.Health <= 0.0f)
			{
				Instance.bIsDestroyed = true;
				Instance.TimeUntilRegrowth = Instance.RegrowthTime;

				// Find the config for this foliage type
				int32 ConfigIndex = FoliageTypeIndices.FindRef(Instance.FoliageType);
				if (ConfigIndex < FoliageConfigs.Num())
				{
					const FNaniteFoliageConfig& Config = FoliageConfigs[ConfigIndex];
					
					// Remove from instanced component
					UInstancedStaticMeshComponent* InstancedComponent = FoliageComponents.FindRef(Instance.FoliageType);
					if (InstancedComponent)
					{
						InstancedComponent->RemoveInstance(i);
					}

					// Convert to destructible if available
					if (Config.DestructibleVersion)
					{
						ConvertToDestructible(i, Config);
					}

					// Broadcast destruction event
					OnEnvironmentDestruction.Broadcast(Instance.Transform.GetLocation(), Instance.FoliageType, Config.NutrientValue);
				}
			}
		}
	}

	return bAnyDamage;
}

void ANaniteEnvironment::ConvertToDestructible(int32 InstanceIndex, const FNaniteFoliageConfig& Config)
{
	if (!Config.DestructibleVersion || InstanceIndex >= FoliageInstances.Num())
	{
		return;
	}

	const FFoliageInstanceData& Instance = FoliageInstances[InstanceIndex];

	// Create geometry collection component
	UGeometryCollectionComponent* GeometryComponent = NewObject<UGeometryCollectionComponent>(this);
	if (GeometryComponent)
	{
		GeometryComponent->SetRestCollection(Config.DestructibleVersion);
		GeometryComponent->SetWorldTransform(Instance.Transform);
		GeometryComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		
		// Enable chaos physics
		GeometryComponent->SetSimulatePhysics(true);
		GeometryComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Bind break event
		GeometryComponent->OnChaosBreakEvent.AddDynamic(this, &ANaniteEnvironment::OnDestructibleBreak);

		// Apply initial impulse to make it fall
		FVector ImpulseDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), -1.0f).GetSafeNormal();
		GeometryComponent->AddImpulse(ImpulseDirection * 1000.0f);

		DestructibleComponents.Add(GeometryComponent);

		// Schedule cleanup
		FTimerHandle CleanupTimer;
		GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [this, GeometryComponent]()
		{
			if (IsValid(GeometryComponent))
			{
				DestructibleComponents.Remove(GeometryComponent);
				GeometryComponent->DestroyComponent();
			}
		}, 30.0f, false);
	}
}

void ANaniteEnvironment::DeformTerrain(const FVector& Location, float Radius, float Intensity)
{
	if (!bDynamicTerrain)
	{
		return;
	}

	// For now, this is a placeholder - real terrain deformation would require
	// a more complex system with heightmaps or voxel-based terrain
	UE_LOG(LogTemp, Log, TEXT("Terrain deformation at %s with intensity %f"), *Location.ToString(), Intensity);

	// Apply physics impulse to nearby objects
	TArray<AActor*> NearbyActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Location, Radius, {}, nullptr, {}, NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (PrimComp && PrimComp->IsSimulatingPhysics())
		{
			FVector Direction = (Actor->GetActorLocation() - Location).GetSafeNormal();
			PrimComp->AddImpulse(Direction * Intensity * TerrainDeformationStrength);
		}
	}
}

TArray<FVector> ANaniteEnvironment::GetNearbyNutrientSources(const FVector& Location, float SearchRadius) const
{
	TArray<FVector> NutrientSources;

	for (const FFoliageInstanceData& Instance : FoliageInstances)
	{
		if (!Instance.bIsDestroyed)
		{
			float Distance = FVector::Dist(Instance.Transform.GetLocation(), Location);
			if (Distance <= SearchRadius)
			{
				NutrientSources.Add(Instance.Transform.GetLocation());
			}
		}
	}

	return NutrientSources;
}

float ANaniteEnvironment::ConsumeFoliage(const FVector& Location, float ConsumeRadius)
{
	float TotalNutrients = 0.0f;

	for (int32 i = 0; i < FoliageInstances.Num(); i++)
	{
		FFoliageInstanceData& Instance = FoliageInstances[i];
		
		if (Instance.bIsDestroyed)
		{
			continue;
		}

		float Distance = FVector::Dist(Instance.Transform.GetLocation(), Location);
		if (Distance <= ConsumeRadius)
		{
			// Find the config for nutrient value
			int32 ConfigIndex = FoliageTypeIndices.FindRef(Instance.FoliageType);
			if (ConfigIndex < FoliageConfigs.Num())
			{
				const FNaniteFoliageConfig& Config = FoliageConfigs[ConfigIndex];
				TotalNutrients += Config.NutrientValue;

				// Mark as consumed (destroyed)
				Instance.bIsDestroyed = true;
				Instance.TimeUntilRegrowth = Instance.RegrowthTime;

				// Remove from instanced component
				UInstancedStaticMeshComponent* InstancedComponent = FoliageComponents.FindRef(Instance.FoliageType);
				if (InstancedComponent)
				{
					InstancedComponent->RemoveInstance(i);
				}
			}
		}
	}

	return TotalNutrients;
}

void ANaniteEnvironment::RegenerateDestroyedFoliage()
{
	for (int32 i = 0; i < FoliageInstances.Num(); i++)
	{
		FFoliageInstanceData& Instance = FoliageInstances[i];
		
		if (!Instance.bIsDestroyed)
		{
			continue;
		}

		Instance.TimeUntilRegrowth -= RegrowthCheckInterval * RegrowthRate;
		
		if (Instance.TimeUntilRegrowth <= 0.0f)
		{
			// Regenerate the foliage
			Instance.bIsDestroyed = false;
			Instance.Health = 100.0f;

			// Add back to instanced component
			UInstancedStaticMeshComponent* InstancedComponent = FoliageComponents.FindRef(Instance.FoliageType);
			if (InstancedComponent)
			{
				InstancedComponent->AddInstance(Instance.Transform);
			}
		}
	}
}

void ANaniteEnvironment::ResetEnvironment()
{
	// Clear all foliage instances
	for (auto& ComponentPair : FoliageComponents)
	{
		if (ComponentPair.Value)
		{
			ComponentPair.Value->ClearInstances();
		}
	}

	// Clear destructible components
	for (UGeometryCollectionComponent* Component : DestructibleComponents)
	{
		if (IsValid(Component))
		{
			Component->DestroyComponent();
		}
	}
	DestructibleComponents.Empty();

	// Regenerate everything
	GenerateFoliage();
}

FVector ANaniteEnvironment::GetRandomPositionInRadius(float Radius) const
{
	float Angle = FMath::RandRange(0.0f, 2.0f * PI);
	float Distance = FMath::RandRange(100.0f, Radius);
	
	FVector Offset = FVector(
		FMath::Cos(Angle) * Distance,
		FMath::Sin(Angle) * Distance,
		0.0f
	);

	return GetActorLocation() + Offset;
}

bool ANaniteEnvironment::IsValidSpawnLocation(const FVector& Location) const
{
	// Simple validation - could be expanded to check terrain height, obstacles, etc.
	return true;
}

void ANaniteEnvironment::OnDestructibleBreak(const FChaosBreakEvent& BreakEvent)
{
	// Handle destructible break events
	UE_LOG(LogTemp, Log, TEXT("Destructible broke at location: %s"), *BreakEvent.Location.ToString());
	
	// Could spawn particle effects, sounds, or other environmental responses here
}
