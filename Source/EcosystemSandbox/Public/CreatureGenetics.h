#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "CreatureGenetics.generated.h"

UENUM(BlueprintType)
enum class EMovementType : uint8
{
	Walker		UMETA(DisplayName = "Walker"),
	Flyer		UMETA(DisplayName = "Flyer"),
	Swimmer		UMETA(DisplayName = "Swimmer"),
	Crawler		UMETA(DisplayName = "Crawler"),
	Jumper		UMETA(DisplayName = "Jumper")
};

UENUM(BlueprintType)
enum class EAggressionLevel : uint8
{
	Peaceful	UMETA(DisplayName = "Peaceful"),
	Defensive	UMETA(DisplayName = "Defensive"),
	Territorial	UMETA(DisplayName = "Territorial"),
	Aggressive	UMETA(DisplayName = "Aggressive"),
	Predatory	UMETA(DisplayName = "Predatory")
};

USTRUCT(BlueprintType)
struct ECOSYSTEMSANDBOX_API FCreatureGenes
{
	GENERATED_BODY()

	// Physical Traits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float Size = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float Speed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float Strength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float Endurance = 1.0f;

	// Behavioral Traits
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMovementType MovementType = EMovementType::Walker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAggressionLevel AggressionLevel = EAggressionLevel::Peaceful;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ReproductionRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float LifeSpan = 30.0f;

	// Intelligence Traits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float Intelligence = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float VisionRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float SocialTendency = 1.0f;

	// Survival Traits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float HungerResistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float DamageResistance = 1.0f;

	// Color/Appearance (for visual variety)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Gray;

	FCreatureGenes()
	{
		// Default constructor
	}

	// Mutation function
	void Mutate(float MutationRate = 0.1f, float MutationStrength = 0.2f);

	// Crossover function for reproduction
	static FCreatureGenes Crossover(const FCreatureGenes& Parent1, const FCreatureGenes& Parent2);

	// Calculate fitness score
	float CalculateFitness(float Age, int32 OffspringCount, float ResourcesGathered, int32 CombatWins) const;

	// Generate random genes
	static FCreatureGenes GenerateRandom();
};
