#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CreatureGenetics.h"
#include "EcosystemManager.generated.h"

class ACreature;

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FPopulationStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TotalPopulation = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Generation = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AverageFitness = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxFitness = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FCreatureGenes DominantGenes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EMovementType, int32> MovementTypeDistribution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EAggressionLevel, int32> AggressionDistribution;
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FCreatureRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FCreatureGenes Genes;

	UPROPERTY()
	float FitnessScore = 0.0f;

	UPROPERTY()
	float LifeTime = 0.0f;

	UPROPERTY()
	int32 Generation = 0;

	UPROPERTY()
	int32 OffspringCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationComplete, int32, GenerationNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPopulationStatsUpdated, FPopulationStats, Stats, float, DeltaTime);

UCLASS()
class ECOSYSTEMSANDBOX_API AEcosystemManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AEcosystemManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Ecosystem Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
	int32 InitialPopulation = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
	int32 MaxPopulation = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
	int32 MinPopulation = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
	float SpawnRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
	TSubclassOf<ACreature> CreatureClass;

	// Genetic Algorithm Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
	float MutationRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
	float MutationStrength = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
	float ElitePercentage = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
	bool bAutoGenerationAdvance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
	float GenerationTimeLimit = 300.0f; // 5 minutes

	// Current State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
	FPopulationStats CurrentStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
	TArray<ACreature*> ActiveCreatures;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
	TArray<FCreatureRecord> CreatureHistory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
	float CurrentGenerationTime = 0.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGenerationComplete OnGenerationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPopulationStatsUpdated OnPopulationStatsUpdated;

public:
	// Public Functions
	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void InitializeEcosystem();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void ResetEcosystem();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void AdvanceGeneration();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	ACreature* SpawnCreature(const FCreatureGenes& Genes, const FVector& Location = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void RemoveCreature(ACreature* Creature);

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	FPopulationStats GetCurrentStats() const { return CurrentStats; }

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	TArray<FCreatureGenes> GetTopPerformers(int32 Count = 10) const;

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void SaveGenerationData();

	UFUNCTION(BlueprintCallable, Category = "Ecosystem")
	void LoadGenerationData();

protected:
	// Internal Functions
	void UpdatePopulationStats();
	void CheckGenerationConditions();
	FVector GetRandomSpawnLocation() const;
	TArray<FCreatureGenes> SelectParentsForReproduction() const;
	void CleanupDeadCreatures();

	UFUNCTION()
	void OnCreatureDeath(ACreature* DeadCreature, float FitnessScore);

	UFUNCTION()
	void OnCreatureReproduction(ACreature* Parent1, ACreature* Parent2, FCreatureGenes OffspringGenes);

private:
	// Performance tracking
	float LastStatsUpdateTime = 0.0f;
	float StatsUpdateInterval = 1.0f;

	// Generation management
	bool bGenerationInProgress = false;
	TArray<FCreatureGenes> PendingOffspring;
};
