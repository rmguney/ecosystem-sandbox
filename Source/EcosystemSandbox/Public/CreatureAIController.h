#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "CreatureAIController.generated.h"

class ACreature;

UCLASS()
class ECOSYSTEMSANDBOX_API ACreatureAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACreatureAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComponent;

public:
	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName TargetLocationKey = TEXT("TargetLocation");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName StateKey = TEXT("CreatureState");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName EnergyKey = TEXT("Energy");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName HungerKey = TEXT("Hunger");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName CanReproduceKey = TEXT("CanReproduce");

	// AI Functions
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateCreatureState(const FString& NewState);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateStats(float Energy, float Hunger, bool bCanReproduce);

protected:
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
	ACreature* ControlledCreature;

	// Perception setup
	void SetupPerception();
};
