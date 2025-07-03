#include "Creature.h"
#include "CreatureAIController.h"
#include "DynamicWeatherSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

ACreature::ACreature()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup interaction sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(100.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Setup body mesh
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetMesh());

	// Set default AI controller
	AIControllerClass = ACreatureAIController::StaticClass();

	// Enable physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);

	// Setup chaos physics properties
	GetMesh()->SetUseCCD(true);
	GetMesh()->SetNotifyRigidBodyCollision(true);
}

void ACreature::BeginPlay()
{
	Super::BeginPlay();

	CreatureAI = Cast<ACreatureAIController>(GetController());

	// Find weather system
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicWeatherSystem::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		WeatherSystem = Cast<ADynamicWeatherSystem>(FoundActors[0]);
	}

	// Bind overlap events
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACreature::OnInteractionSphereBeginOverlap);

	// Apply genetic traits
	ApplyGeneticTraits();
	UpdateAppearance();
	UpdateMovementParameters();
}

void ACreature::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLifeStats(DeltaTime);
	UpdateWeatherEffects(DeltaTime);

	// Check if creature should die
	if (Health <= 0.0f || Age >= Genes.LifeSpan)
	{
		Die();
	}

	// Update AI with current stats
	if (CreatureAI)
	{
		CreatureAI->UpdateStats(Energy, Hunger, CanReproduce());
	}
}

bool ACreature::CanReproduce() const
{
	return Age >= MaturityAge && 
		   Energy > 50.0f && 
		   Hunger < 70.0f && 
		   ReproductionCooldown <= 0.0f &&
		   Health > 30.0f;
}

float ACreature::CalculateFitnessScore() const
{
	return Genes.CalculateFitness(Age, OffspringCount, ResourcesGathered, CombatWins);
}

void ACreature::SetCreatureState(ECreatureState NewState)
{
	CurrentState = NewState;
	
	if (CreatureAI)
	{
		FString StateString = UEnum::GetValueAsString(NewState);
		CreatureAI->UpdateCreatureState(StateString);
	}
}

void ACreature::InitializeWithGenes(const FCreatureGenes& NewGenes)
{
	Genes = NewGenes;
	ApplyGeneticTraits();
	UpdateAppearance();
	UpdateMovementParameters();
}

bool ACreature::AttemptReproduction(ACreature* Mate)
{
	if (!Mate || !CanReproduce() || !Mate->CanReproduce())
	{
		return false;
	}

	// Create offspring genes
	FCreatureGenes OffspringGenes = FCreatureGenes::Crossover(Genes, Mate->Genes);
	OffspringGenes.Mutate(0.1f, 0.2f);

	// Broadcast reproduction event
	OnCreatureReproduction.Broadcast(this, Mate, OffspringGenes);

	// Apply reproduction cost
	Energy -= 30.0f;
	Mate->Energy -= 30.0f;

	// Set cooldown
	ReproductionCooldown = 20.0f / Genes.ReproductionRate;
	Mate->ReproductionCooldown = 20.0f / Mate->Genes.ReproductionRate;

	// Increase offspring count
	OffspringCount++;
	Mate->OffspringCount++;

	return true;
}

void ACreature::TakeDamage(float DamageAmount, ACreature* Attacker)
{
	float ActualDamage = DamageAmount / Genes.DamageResistance;
	Health = FMath::Max(0.0f, Health - ActualDamage);

	// Apply physics impulse for impact
	if (GetMesh() && Attacker)
	{
		FVector ImpulseDirection = (GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
		float ImpulseStrength = ActualDamage * 100.0f;
		GetMesh()->AddImpulse(ImpulseDirection * ImpulseStrength);
	}

	if (Health <= 0.0f)
	{
		Die();
		if (Attacker)
		{
			Attacker->CombatWins++;
		}
	}
}

void ACreature::ConsumeFood(float FoodValue)
{
	Energy = FMath::Min(100.0f, Energy + FoodValue);
	Hunger = FMath::Max(0.0f, Hunger - FoodValue * 0.5f);
	ResourcesGathered += FoodValue;
}

void ACreature::Attack(ACreature* Target)
{
	if (!Target || FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > 200.0f)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < 1.0f)
	{
		return;
	}

	LastAttackTime = CurrentTime;

	float AttackDamage = Genes.Strength * 10.0f;
	Target->TakeDamage(AttackDamage, this);

	// Apply attack physics
	FVector AttackDirection = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (Target->GetMesh())
	{
		Target->GetMesh()->AddImpulse(AttackDirection * Genes.Strength * 500.0f);
	}

	Energy -= 5.0f;
}

void ACreature::Die()
{
	SetCreatureState(ECreatureState::Dying);
	
	float FitnessScore = CalculateFitnessScore();
	OnCreatureDeath.Broadcast(this, FitnessScore);

	// Disable AI
	if (CreatureAI)
	{
		CreatureAI->GetBrainComponent()->StopLogic(TEXT("Death"));
	}

	// Schedule destruction
	SetLifeSpan(2.0f);
}

void ACreature::UpdateLifeStats(float DeltaTime)
{
	// Age the creature
	Age += DeltaTime * AgingRate;

	// Energy decay
	float EnergyDecay = EnergyDecayRate * DeltaTime;
	Energy = FMath::Max(0.0f, Energy - EnergyDecay);

	// Hunger growth
	float HungerGrowth = HungerGrowthRate * DeltaTime / Genes.HungerResistance;
	Hunger = FMath::Min(100.0f, Hunger + HungerGrowth);

	// Reduce reproduction cooldown
	if (ReproductionCooldown > 0.0f)
	{
		ReproductionCooldown -= DeltaTime;
	}

	// Health effects
	if (Energy <= 0.0f || Hunger >= 90.0f)
	{
		Health -= 10.0f * DeltaTime;
	}
	else if (Energy > 70.0f && Hunger < 30.0f)
	{
		Health = FMath::Min(100.0f, Health + 5.0f * DeltaTime);
	}
}

void ACreature::UpdateAppearance()
{
	if (!BodyMesh || !BodyMesh->GetMaterial(0))
	{
		return;
	}

	// Create dynamic material instance
	UMaterialInstanceDynamic* DynamicMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), Genes.PrimaryColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("SecondaryColor"), Genes.SecondaryColor);
	}

	// Scale creature based on size gene
	FVector Scale = FVector(Genes.Size);
	SetActorScale3D(Scale);

	// Update interaction sphere size
	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(100.0f * Genes.Size);
	}
}

void ACreature::UpdateMovementParameters()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Update movement based on genes
	MovementComp->MaxWalkSpeed = Genes.Speed * 200.0f;
	MovementComp->JumpZVelocity = Genes.Strength * 400.0f;
	MovementComp->Mass = Genes.Size * 100.0f;

	// Movement type specific adjustments
	switch (Genes.MovementType)
	{
		case EMovementType::Flyer:
			MovementComp->SetMovementMode(MOVE_Flying);
			MovementComp->MaxFlySpeed = Genes.Speed * 300.0f;
			break;
		case EMovementType::Swimmer:
			MovementComp->SetMovementMode(MOVE_Swimming);
			MovementComp->MaxSwimSpeed = Genes.Speed * 250.0f;
			break;
		case EMovementType::Jumper:
			MovementComp->JumpZVelocity = Genes.Strength * 600.0f;
			break;
		default:
			MovementComp->SetMovementMode(MOVE_Walking);
			break;
	}
}

void ACreature::ApplyGeneticTraits()
{
	// Update life parameters based on genes
	EnergyDecayRate = 2.0f / Genes.Endurance;
	HungerGrowthRate = 2.0f / Genes.HungerResistance;
	AgingRate = 1.0f / Genes.LifeSpan * 100.0f;
	MaturityAge = Genes.LifeSpan * 0.2f;

	// Update AI perception range
	if (CreatureAI && CreatureAI->GetAIPerceptionComponent())
	{
		// This would require updating the sight config at runtime
		// For now, we'll just store the vision range
	}
}

void ACreature::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACreature* OtherCreature = Cast<ACreature>(OtherActor);
	if (!OtherCreature)
	{
		return;
	}

	// Handle creature interactions based on aggression and state
	switch (Genes.AggressionLevel)
	{
		case EAggressionLevel::Aggressive:
		case EAggressionLevel::Predatory:
			if (CurrentState != ECreatureState::Mating && Energy > 30.0f)
			{
				SetCreatureState(ECreatureState::Fighting);
				Attack(OtherCreature);
			}
			break;

		case EAggressionLevel::Peaceful:
			if (CanReproduce() && OtherCreature->CanReproduce())
			{
				SetCreatureState(ECreatureState::Mating);
				AttemptReproduction(OtherCreature);
			}
			break;

		default:
			// Defensive or territorial behavior
			if (Genes.SocialTendency > 1.5f && CanReproduce() && OtherCreature->CanReproduce())
			{
				SetCreatureState(ECreatureState::Mating);
				AttemptReproduction(OtherCreature);
			}
			break;
	}
}

void ACreature::UpdateWeatherEffects(float DeltaTime)
{
	if (!WeatherSystem)
	{
		return;
	}

	// Get weather effects
	float TemperatureEffect = WeatherSystem->GetTemperatureEffect();
	float VisibilityModifier = WeatherSystem->GetVisibilityModifier();
	float MovementModifier = WeatherSystem->GetMovementModifier();

	// Apply temperature effects on energy consumption
	float TemperatureStress = FMath::Abs(TemperatureEffect - 0.5f) * 2.0f; // 0 to 1 scale
	float AdditionalEnergyDecay = TemperatureStress * 0.5f * DeltaTime;
	Energy = FMath::Max(0.0f, Energy - AdditionalEnergyDecay);

	// Apply movement modifier to character movement
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		float BaseSpeed = Genes.Speed * 200.0f;
		MovementComp->MaxWalkSpeed = BaseSpeed * MovementModifier;
	}

	// Apply visibility effects to AI perception
	if (CreatureAI && CreatureAI->GetAIPerceptionComponent())
	{
		// Reduce perception range in bad weather
		// This would require updating sight config at runtime
		// For now, we'll affect the creature's behavior through energy
		if (VisibilityModifier < 0.5f)
		{
			Energy -= 0.2f * DeltaTime; // Extra energy cost in poor visibility
		}
	}

	// Weather affects health regeneration
	if (TemperatureEffect > 0.3f && TemperatureEffect < 0.8f) // Comfortable temperature range
	{
		if (Energy > 70.0f && Hunger < 30.0f)
		{
			Health = FMath::Min(100.0f, Health + 6.0f * DeltaTime); // Faster healing in good weather
		}
	}
}
