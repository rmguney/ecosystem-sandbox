#include "EcosystemManager.h"
#include "Creature.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AEcosystemManager::AEcosystemManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void AEcosystemManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeEcosystem();
}

void AEcosystemManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentGenerationTime += DeltaTime;

	// Update stats periodically
	if (GetWorld()->GetTimeSeconds() - LastStatsUpdateTime > StatsUpdateInterval)
	{
		UpdatePopulationStats();
		OnPopulationStatsUpdated.Broadcast(CurrentStats, DeltaTime);
		LastStatsUpdateTime = GetWorld()->GetTimeSeconds();
	}

	// Check generation advancement conditions
	if (bAutoGenerationAdvance)
	{
		CheckGenerationConditions();
	}

	// Clean up dead creatures
	CleanupDeadCreatures();
}

void AEcosystemManager::InitializeEcosystem()
{
	if (!CreatureClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreatureClass not set in EcosystemManager!"));
		return;
	}

	// Clear existing data
	ActiveCreatures.Empty();
	CreatureHistory.Empty();
	PendingOffspring.Empty();

	// Reset stats
	CurrentStats = FPopulationStats();
	CurrentGenerationTime = 0.0f;

	// Spawn initial population with random genes
	for (int32 i = 0; i < InitialPopulation; i++)
	{
		FCreatureGenes RandomGenes = FCreatureGenes::GenerateRandom();
		FVector SpawnLocation = GetRandomSpawnLocation();
		SpawnCreature(RandomGenes, SpawnLocation);
	}

	UE_LOG(LogTemp, Warning, TEXT("Ecosystem initialized with %d creatures"), InitialPopulation);
}

void AEcosystemManager::ResetEcosystem()
{
	// Destroy all existing creatures
	for (ACreature* Creature : ActiveCreatures)
	{
		if (IsValid(Creature))
		{
			Creature->Destroy();
		}
	}

	InitializeEcosystem();
}

void AEcosystemManager::AdvanceGeneration()
{
	if (bGenerationInProgress)
	{
		return;
	}

	bGenerationInProgress = true;
	CurrentStats.Generation++;

	UE_LOG(LogTemp, Warning, TEXT("Advancing to Generation %d"), CurrentStats.Generation);

	// Get top performers for the next generation
	TArray<FCreatureGenes> ParentGenes = SelectParentsForReproduction();
	
	// Destroy current population
	for (ACreature* Creature : ActiveCreatures)
	{
		if (IsValid(Creature))
		{
			Creature->Destroy();
		}
	}
	ActiveCreatures.Empty();

	// Create new generation
	int32 EliteCount = FMath::RoundToInt(ParentGenes.Num() * ElitePercentage);
	
	// Keep elite unchanged
	for (int32 i = 0; i < EliteCount; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		SpawnCreature(ParentGenes[i], SpawnLocation);
	}

	// Create offspring from crossover and mutation
	while (ActiveCreatures.Num() < InitialPopulation)
	{
		// Select two random parents
		int32 Parent1Index = FMath::RandRange(0, ParentGenes.Num() - 1);
		int32 Parent2Index = FMath::RandRange(0, ParentGenes.Num() - 1);

		// Ensure different parents
		while (Parent2Index == Parent1Index && ParentGenes.Num() > 1)
		{
			Parent2Index = FMath::RandRange(0, ParentGenes.Num() - 1);
		}

		// Create offspring
		FCreatureGenes OffspringGenes = FCreatureGenes::Crossover(ParentGenes[Parent1Index], ParentGenes[Parent2Index]);
		OffspringGenes.Mutate(MutationRate, MutationStrength);

		FVector SpawnLocation = GetRandomSpawnLocation();
		SpawnCreature(OffspringGenes, SpawnLocation);
	}

	// Reset generation timer
	CurrentGenerationTime = 0.0f;
	bGenerationInProgress = false;

	OnGenerationComplete.Broadcast(CurrentStats.Generation);
}

ACreature* AEcosystemManager::SpawnCreature(const FCreatureGenes& Genes, const FVector& Location)
{
	if (!CreatureClass)
	{
		return nullptr;
	}

	FVector SpawnLocation = Location;
	if (SpawnLocation == FVector::ZeroVector)
	{
		SpawnLocation = GetRandomSpawnLocation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACreature* NewCreature = GetWorld()->SpawnActor<ACreature>(CreatureClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	
	if (NewCreature)
	{
		NewCreature->InitializeWithGenes(Genes);
		NewCreature->OnCreatureDeath.AddDynamic(this, &AEcosystemManager::OnCreatureDeath);
		NewCreature->OnCreatureReproduction.AddDynamic(this, &AEcosystemManager::OnCreatureReproduction);
		
		ActiveCreatures.Add(NewCreature);
	}

	return NewCreature;
}

void AEcosystemManager::RemoveCreature(ACreature* Creature)
{
	if (Creature)
	{
		ActiveCreatures.Remove(Creature);
	}
}

TArray<FCreatureGenes> AEcosystemManager::GetTopPerformers(int32 Count) const
{
	TArray<FCreatureRecord> SortedRecords = CreatureHistory;
	
	// Sort by fitness score
	SortedRecords.Sort([](const FCreatureRecord& A, const FCreatureRecord& B)
	{
		return A.FitnessScore > B.FitnessScore;
	});

	TArray<FCreatureGenes> TopGenes;
	for (int32 i = 0; i < FMath::Min(Count, SortedRecords.Num()); i++)
	{
		TopGenes.Add(SortedRecords[i].Genes);
	}

	return TopGenes;
}

void AEcosystemManager::SaveGenerationData()
{
	// This would typically save to a file or database
	// For now, just log the data
	UE_LOG(LogTemp, Warning, TEXT("Generation %d Data - Population: %d, Avg Fitness: %.2f, Max Fitness: %.2f"), 
		CurrentStats.Generation, CurrentStats.TotalPopulation, CurrentStats.AverageFitness, CurrentStats.MaxFitness);
}

void AEcosystemManager::LoadGenerationData()
{
	// This would typically load from a file or database
	UE_LOG(LogTemp, Warning, TEXT("LoadGenerationData - Not implemented yet"));
}

void AEcosystemManager::UpdatePopulationStats()
{
	CurrentStats.TotalPopulation = ActiveCreatures.Num();

	if (CurrentStats.TotalPopulation == 0)
	{
		return;
	}

	// Calculate fitness statistics
	float TotalFitness = 0.0f;
	float MaxFitness = 0.0f;
	FCreatureGenes BestGenes;

	// Reset distribution maps
	CurrentStats.MovementTypeDistribution.Empty();
	CurrentStats.AggressionDistribution.Empty();

	for (ACreature* Creature : ActiveCreatures)
	{
		if (!IsValid(Creature))
		{
			continue;
		}

		float CreatureFitness = Creature->CalculateFitnessScore();
		TotalFitness += CreatureFitness;

		if (CreatureFitness > MaxFitness)
		{
			MaxFitness = CreatureFitness;
			BestGenes = Creature->GetGenes();
		}

		// Update distributions
		FCreatureGenes Genes = Creature->GetGenes();
		CurrentStats.MovementTypeDistribution.FindOrAdd(Genes.MovementType)++;
		CurrentStats.AggressionDistribution.FindOrAdd(Genes.AggressionLevel)++;
	}

	CurrentStats.AverageFitness = TotalFitness / CurrentStats.TotalPopulation;
	CurrentStats.MaxFitness = MaxFitness;
	CurrentStats.DominantGenes = BestGenes;
}

void AEcosystemManager::CheckGenerationConditions()
{
	bool bShouldAdvance = false;

	// Check time limit
	if (CurrentGenerationTime >= GenerationTimeLimit)
	{
		bShouldAdvance = true;
		UE_LOG(LogTemp, Warning, TEXT("Advancing generation due to time limit"));
	}

	// Check population thresholds
	if (CurrentStats.TotalPopulation <= MinPopulation)
	{
		bShouldAdvance = true;
		UE_LOG(LogTemp, Warning, TEXT("Advancing generation due to low population: %d"), CurrentStats.TotalPopulation);
	}

	if (CurrentStats.TotalPopulation >= MaxPopulation)
	{
		bShouldAdvance = true;
		UE_LOG(LogTemp, Warning, TEXT("Advancing generation due to high population: %d"), CurrentStats.TotalPopulation);
	}

	if (bShouldAdvance)
	{
		AdvanceGeneration();
	}
}

FVector AEcosystemManager::GetRandomSpawnLocation() const
{
	float Angle = FMath::RandRange(0.0f, 2.0f * PI);
	float Distance = FMath::RandRange(100.0f, SpawnRadius);
	
	FVector Offset = FVector(
		FMath::Cos(Angle) * Distance,
		FMath::Sin(Angle) * Distance,
		0.0f
	);

	return GetActorLocation() + Offset;
}

TArray<FCreatureGenes> AEcosystemManager::SelectParentsForReproduction() const
{
	TArray<FCreatureRecord> CurrentRecords;

	// Create records for current population
	for (ACreature* Creature : ActiveCreatures)
	{
		if (!IsValid(Creature))
		{
			continue;
		}

		FCreatureRecord Record;
		Record.Genes = Creature->GetGenes();
		Record.FitnessScore = Creature->CalculateFitnessScore();
		Record.LifeTime = Creature->GetAge();
		Record.Generation = CurrentStats.Generation;
		Record.OffspringCount = Creature->OffspringCount;

		CurrentRecords.Add(Record);
	}

	// Sort by fitness
	CurrentRecords.Sort([](const FCreatureRecord& A, const FCreatureRecord& B)
	{
		return A.FitnessScore > B.FitnessScore;
	});

	// Select top performers
	int32 ParentCount = FMath::Max(2, FMath::RoundToInt(CurrentRecords.Num() * 0.5f));
	TArray<FCreatureGenes> ParentGenes;

	for (int32 i = 0; i < FMath::Min(ParentCount, CurrentRecords.Num()); i++)
	{
		ParentGenes.Add(CurrentRecords[i].Genes);
	}

	return ParentGenes;
}

void AEcosystemManager::CleanupDeadCreatures()
{
	for (int32 i = ActiveCreatures.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(ActiveCreatures[i]))
		{
			ActiveCreatures.RemoveAt(i);
		}
	}
}

void AEcosystemManager::OnCreatureDeath(ACreature* DeadCreature, float FitnessScore)
{
	if (!DeadCreature)
	{
		return;
	}

	// Record creature data for analysis
	FCreatureRecord Record;
	Record.Genes = DeadCreature->GetGenes();
	Record.FitnessScore = FitnessScore;
	Record.LifeTime = DeadCreature->GetAge();
	Record.Generation = CurrentStats.Generation;
	Record.OffspringCount = DeadCreature->OffspringCount;

	CreatureHistory.Add(Record);

	// Remove from active list
	RemoveCreature(DeadCreature);
}

void AEcosystemManager::OnCreatureReproduction(ACreature* Parent1, ACreature* Parent2, FCreatureGenes OffspringGenes)
{
	// Spawn new creature immediately or queue for later
	if (ActiveCreatures.Num() < MaxPopulation)
	{
		FVector SpawnLocation = (Parent1->GetActorLocation() + Parent2->GetActorLocation()) * 0.5f;
		SpawnLocation += FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
		
		SpawnCreature(OffspringGenes, SpawnLocation);
	}
}
