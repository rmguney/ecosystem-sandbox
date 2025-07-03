#include "CreatureGenetics.h"
#include "Math/UnrealMathUtility.h"

void FCreatureGenes::Mutate(float MutationRate, float MutationStrength)
{
	auto MutateFloat = [&](float& Value, float Min, float Max)
	{
		if (FMath::RandRange(0.0f, 1.0f) < MutationRate)
		{
			float Change = FMath::RandRange(-MutationStrength, MutationStrength);
			Value = FMath::Clamp(Value + Change, Min, Max);
		}
	};

	auto MutateEnum = [&](auto& EnumValue, int32 MaxValue)
	{
		if (FMath::RandRange(0.0f, 1.0f) < MutationRate)
		{
			EnumValue = static_cast<decltype(EnumValue)>(FMath::RandRange(0, MaxValue));
		}
	};

	// Mutate physical traits
	MutateFloat(Size, 0.1f, 5.0f);
	MutateFloat(Speed, 0.1f, 10.0f);
	MutateFloat(Strength, 0.1f, 3.0f);
	MutateFloat(Endurance, 0.1f, 2.0f);

	// Mutate behavioral traits
	MutateEnum(MovementType, 4);
	MutateEnum(AggressionLevel, 4);
	MutateFloat(ReproductionRate, 0.01f, 1.0f);
	MutateFloat(LifeSpan, 1.0f, 100.0f);

	// Mutate intelligence traits
	MutateFloat(Intelligence, 0.1f, 3.0f);
	MutateFloat(VisionRange, 100.0f, 2000.0f);
	MutateFloat(SocialTendency, 0.1f, 2.0f);

	// Mutate survival traits
	MutateFloat(HungerResistance, 0.1f, 3.0f);
	MutateFloat(DamageResistance, 0.1f, 3.0f);

	// Mutate colors
	if (FMath::RandRange(0.0f, 1.0f) < MutationRate)
	{
		PrimaryColor = FLinearColor::MakeRandomColor();
	}
	if (FMath::RandRange(0.0f, 1.0f) < MutationRate)
	{
		SecondaryColor = FLinearColor::MakeRandomColor();
	}
}

FCreatureGenes FCreatureGenes::Crossover(const FCreatureGenes& Parent1, const FCreatureGenes& Parent2)
{
	FCreatureGenes Offspring;

	auto BlendFloat = [](float A, float B) -> float
	{
		return FMath::RandRange(0.0f, 1.0f) < 0.5f ? A : B;
	};

	auto BlendEnum = [](auto A, auto B) -> decltype(A)
	{
		return FMath::RandRange(0.0f, 1.0f) < 0.5f ? A : B;
	};

	// Blend physical traits
	Offspring.Size = BlendFloat(Parent1.Size, Parent2.Size);
	Offspring.Speed = BlendFloat(Parent1.Speed, Parent2.Speed);
	Offspring.Strength = BlendFloat(Parent1.Strength, Parent2.Strength);
	Offspring.Endurance = BlendFloat(Parent1.Endurance, Parent2.Endurance);

	// Blend behavioral traits
	Offspring.MovementType = BlendEnum(Parent1.MovementType, Parent2.MovementType);
	Offspring.AggressionLevel = BlendEnum(Parent1.AggressionLevel, Parent2.AggressionLevel);
	Offspring.ReproductionRate = BlendFloat(Parent1.ReproductionRate, Parent2.ReproductionRate);
	Offspring.LifeSpan = BlendFloat(Parent1.LifeSpan, Parent2.LifeSpan);

	// Blend intelligence traits
	Offspring.Intelligence = BlendFloat(Parent1.Intelligence, Parent2.Intelligence);
	Offspring.VisionRange = BlendFloat(Parent1.VisionRange, Parent2.VisionRange);
	Offspring.SocialTendency = BlendFloat(Parent1.SocialTendency, Parent2.SocialTendency);

	// Blend survival traits
	Offspring.HungerResistance = BlendFloat(Parent1.HungerResistance, Parent2.HungerResistance);
	Offspring.DamageResistance = BlendFloat(Parent1.DamageResistance, Parent2.DamageResistance);

	// Blend colors
	Offspring.PrimaryColor = FMath::RandRange(0.0f, 1.0f) < 0.5f ? Parent1.PrimaryColor : Parent2.PrimaryColor;
	Offspring.SecondaryColor = FMath::RandRange(0.0f, 1.0f) < 0.5f ? Parent1.SecondaryColor : Parent2.SecondaryColor;

	return Offspring;
}

float FCreatureGenes::CalculateFitness(float Age, int32 OffspringCount, float ResourcesGathered, int32 CombatWins) const
{
	float Fitness = 0.0f;

	// Survival bonus (lived longer = better)
	Fitness += Age / LifeSpan * 100.0f;

	// Reproduction success (more offspring = better)
	Fitness += OffspringCount * 50.0f;

	// Resource gathering efficiency
	Fitness += ResourcesGathered * 0.1f;

	// Combat effectiveness
	Fitness += CombatWins * 25.0f;

	// Size efficiency (not too big, not too small)
	float SizeOptimal = FMath::Abs(Size - 1.0f);
	Fitness += (1.0f - SizeOptimal) * 20.0f;

	// Intelligence bonus
	Fitness += Intelligence * 15.0f;

	// Endurance bonus
	Fitness += Endurance * 10.0f;

	return FMath::Max(0.0f, Fitness);
}

FCreatureGenes FCreatureGenes::GenerateRandom()
{
	FCreatureGenes RandomGenes;

	RandomGenes.Size = FMath::RandRange(0.1f, 5.0f);
	RandomGenes.Speed = FMath::RandRange(0.1f, 10.0f);
	RandomGenes.Strength = FMath::RandRange(0.1f, 3.0f);
	RandomGenes.Endurance = FMath::RandRange(0.1f, 2.0f);

	RandomGenes.MovementType = static_cast<EMovementType>(FMath::RandRange(0, 4));
	RandomGenes.AggressionLevel = static_cast<EAggressionLevel>(FMath::RandRange(0, 4));
	RandomGenes.ReproductionRate = FMath::RandRange(0.01f, 1.0f);
	RandomGenes.LifeSpan = FMath::RandRange(1.0f, 100.0f);

	RandomGenes.Intelligence = FMath::RandRange(0.1f, 3.0f);
	RandomGenes.VisionRange = FMath::RandRange(100.0f, 2000.0f);
	RandomGenes.SocialTendency = FMath::RandRange(0.1f, 2.0f);

	RandomGenes.HungerResistance = FMath::RandRange(0.1f, 3.0f);
	RandomGenes.DamageResistance = FMath::RandRange(0.1f, 3.0f);

	RandomGenes.PrimaryColor = FLinearColor::MakeRandomColor();
	RandomGenes.SecondaryColor = FLinearColor::MakeRandomColor();

	return RandomGenes;
}
