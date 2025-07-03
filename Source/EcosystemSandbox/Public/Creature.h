#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "CreatureGenetics.h"
#include "Creature.generated.h"

class ACreatureAIController;
class ADynamicWeatherSystem;

UENUM(BlueprintType)
enum class ECreatureState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Wandering	UMETA(DisplayName = "Wandering"),
	Seeking		UMETA(DisplayName = "Seeking"),
	Feeding		UMETA(DisplayName = "Feeding"),
	Fleeing		UMETA(DisplayName = "Fleeing"),
	Fighting	UMETA(DisplayName = "Fighting"),
	Mating		UMETA(DisplayName = "Mating"),
	Dying		UMETA(DisplayName = "Dying")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreatureDeath, ACreature*, DeadCreature, float, FitnessScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreatureReproduction, ACreature*, Parent1, ACreature*, Parent2, FCreatureGenes, OffspringGenes);

UCLASS()
class ECOSYSTEMSANDBOX_API ACreature : public ACharacter
{
	GENERATED_BODY()

public:
	ACreature();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BodyMesh;

	// Genetics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetics")
	FCreatureGenes Genes;

	// Life Stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life Stats")
	float Energy = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life Stats")
	float Hunger = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life Stats")
	float Age = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life Stats")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Life Stats")
	ECreatureState CurrentState = ECreatureState::Idle;

	// Reproduction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reproduction")
	float MaturityAge = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reproduction")
	float ReproductionCooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reproduction")
	int32 OffspringCount = 0;

	// Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 CombatWins = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float LastAttackTime = 0.0f;

	// Resources
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
	float ResourcesGathered = 0.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCreatureDeath OnCreatureDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCreatureReproduction OnCreatureReproduction;

public:
	// Getters
	UFUNCTION(BlueprintCallable, Category = "Creature")
	float GetEnergy() const { return Energy; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	float GetHunger() const { return Hunger; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	float GetAge() const { return Age; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	ECreatureState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	FCreatureGenes GetGenes() const { return Genes; }

	UFUNCTION(BlueprintCallable, Category = "Creature")
	bool CanReproduce() const;

	UFUNCTION(BlueprintCallable, Category = "Creature")
	float CalculateFitnessScore() const;

	// Actions
	UFUNCTION(BlueprintCallable, Category = "Creature")
	void SetCreatureState(ECreatureState NewState);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void InitializeWithGenes(const FCreatureGenes& NewGenes);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	bool AttemptReproduction(ACreature* Mate);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void TakeDamage(float DamageAmount, ACreature* Attacker = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void ConsumeFood(float FoodValue);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void Attack(ACreature* Target);

	UFUNCTION(BlueprintCallable, Category = "Creature")
	void Die();

protected:
	// Internal functions
	void UpdateLifeStats(float DeltaTime);
	void UpdateAppearance();
	void UpdateMovementParameters();
	void ApplyGeneticTraits();
	void UpdateWeatherEffects(float DeltaTime);

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	ACreatureAIController* CreatureAI;
	ADynamicWeatherSystem* WeatherSystem;
	
	// Cached values for performance
	float EnergyDecayRate = 1.0f;
	float HungerGrowthRate = 1.5f;
	float AgingRate = 1.0f;
};
