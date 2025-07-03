#include "BTTask_FindFood.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Creature.h"
#include "CreatureAIController.h"
#include "NaniteEnvironment.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FindFood::UBTTask_FindFood()
{
	NodeName = "Find Food";
	bNotifyTick = false;
	bNotifyTaskFinished = true;

	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindFood, TargetLocationKey));
}

EBTNodeResult::Type UBTTask_FindFood::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACreature* Creature = GetCreature(OwnerComp);
	if (!Creature)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	FVector CreatureLocation = Creature->GetActorLocation();
	
	// Find nearest environment
	ANaniteEnvironment* Environment = FindNearestEnvironment(CreatureLocation);
	if (!Environment)
	{
		return EBTNodeResult::Failed;
	}

	// Get nearby nutrient sources
	TArray<FVector> NutrientSources = Environment->GetNearbyNutrientSources(CreatureLocation, SearchRadius);
	
	if (NutrientSources.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	// Find closest food source
	FVector ClosestFood = NutrientSources[0];
	float ClosestDistance = FVector::Dist(CreatureLocation, ClosestFood);

	for (const FVector& FoodLocation : NutrientSources)
	{
		float Distance = FVector::Dist(CreatureLocation, FoodLocation);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestFood = FoodLocation;
		}
	}

	// Set target location in blackboard
	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, ClosestFood);

	return EBTNodeResult::Succeeded;
}

ACreature* UBTTask_FindFood::GetCreature(UBehaviorTreeComponent& OwnerComp) const
{
	ACreatureAIController* AIController = Cast<ACreatureAIController>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		return Cast<ACreature>(AIController->GetPawn());
	}
	return nullptr;
}

ANaniteEnvironment* UBTTask_FindFood::FindNearestEnvironment(const FVector& Location) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ANaniteEnvironment::StaticClass(), FoundActors);

	ANaniteEnvironment* NearestEnvironment = nullptr;
	float NearestDistance = FLT_MAX;

	for (AActor* Actor : FoundActors)
	{
		ANaniteEnvironment* Environment = Cast<ANaniteEnvironment>(Actor);
		if (Environment)
		{
			float Distance = FVector::Dist(Location, Environment->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestEnvironment = Environment;
			}
		}
	}

	return NearestEnvironment;
}
