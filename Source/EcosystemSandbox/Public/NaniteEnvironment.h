#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "NaniteEnvironment.generated.h"

UENUM(BlueprintType)
enum class EFoliageType : uint8
{
	Tree		UMETA(DisplayName = "Tree"),
	Bush		UMETA(DisplayName = "Bush"),
	Grass		UMETA(DisplayName = "Grass"),
	Rock		UMETA(DisplayName = "Rock"),
	Flower		UMETA(DisplayName = "Flower")
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FFoliageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFoliageType FoliageType = EFoliageType::Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RegrowthTime = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilRegrowth = 0.0f;
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FNaniteFoliageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFoliageType FoliageType = EFoliageType::Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMesh* NaniteMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UGeometryCollection* DestructibleVersion = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ScaleRange = FVector(0.8f, 1.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DestructionThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NutrientValue = 25.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnvironmentDestruction, FVector, Location, EFoliageType, Type, float, NutrientValue);

UCLASS()
class ECOSYSTEMSANDBOX_API ANaniteEnvironment : public AActor
{
	GENERATED_BODY()
	
public:	
	ANaniteEnvironment();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Terrain
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* TerrainMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	class UStaticMesh* NaniteTerrainMesh;

	// Foliage Systems
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TMap<EFoliageType, class UInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TArray<FNaniteFoliageConfig> FoliageConfigs;

	// Destructible Environment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<class UGeometryCollectionComponent*> DestructibleComponents;

	// Environment Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Data")
	TArray<FFoliageInstanceData> FoliageInstances;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Configuration")
	float EnvironmentSize = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Configuration")
	bool bEnableRegrowth = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Configuration")
	float RegrowthRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Configuration")
	bool bDynamicTerrain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Configuration")
	float TerrainDeformationStrength = 100.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnvironmentDestruction OnEnvironmentDestruction;

public:
	// Public Functions
	UFUNCTION(BlueprintCallable, Category = "Environment")
	void InitializeEnvironment();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void GenerateFoliage();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	bool DamageFoliage(const FVector& Location, float Radius, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void DeformTerrain(const FVector& Location, float Radius, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	TArray<FVector> GetNearbyNutrientSources(const FVector& Location, float SearchRadius) const;

	UFUNCTION(BlueprintCallable, Category = "Environment")
	float ConsumeFoliage(const FVector& Location, float ConsumeRadius);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void RegenerateDestroyedFoliage();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void ResetEnvironment();

protected:
	// Internal Functions
	void SetupNaniteComponents();
	void SetupFoliageComponents();
	void SpawnFoliageInstance(const FNaniteFoliageConfig& Config, int32 ConfigIndex);
	void ConvertToDestructible(int32 InstanceIndex, const FNaniteFoliageConfig& Config);
	FVector GetRandomPositionInRadius(float Radius) const;
	bool IsValidSpawnLocation(const FVector& Location) const;

	UFUNCTION()
	void OnDestructibleBreak(const FChaosBreakEvent& BreakEvent);

private:
	// Performance optimization
	float LastRegrowthCheck = 0.0f;
	float RegrowthCheckInterval = 5.0f;

	// Cached references for performance
	TMap<EFoliageType, int32> FoliageTypeIndices;
};
