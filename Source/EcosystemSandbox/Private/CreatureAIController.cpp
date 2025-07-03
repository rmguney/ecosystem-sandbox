#include "CreatureAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Creature.h"

ACreatureAIController::ACreatureAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	SetupPerception();
}

void ACreatureAIController::BeginPlay()
{
	Super::BeginPlay();

	ControlledCreature = Cast<ACreature>(GetPawn());

	if (Blackboard && BehaviorTree)
	{
		UseBlackboard(BehaviorTree->BlackboardAsset);
		RunBehaviorTree(BehaviorTree);
	}

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACreatureAIController::OnPerceptionUpdated);
	}
}

void ACreatureAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update blackboard with creature stats
	if (ControlledCreature && BlackboardComponent)
	{
		BlackboardComponent->SetValueAsFloat(EnergyKey, ControlledCreature->GetEnergy());
		BlackboardComponent->SetValueAsFloat(HungerKey, ControlledCreature->GetHunger());
		BlackboardComponent->SetValueAsBool(CanReproduceKey, ControlledCreature->CanReproduce());
	}
}

void ACreatureAIController::SetTargetActor(AActor* NewTarget)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(TargetActorKey, NewTarget);
	}
}

void ACreatureAIController::SetTargetLocation(FVector Location)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsVector(TargetLocationKey, Location);
	}
}

void ACreatureAIController::UpdateCreatureState(const FString& NewState)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsString(StateKey, NewState);
	}
}

void ACreatureAIController::UpdateStats(float Energy, float Hunger, bool bCanReproduce)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsFloat(EnergyKey, Energy);
		BlackboardComponent->SetValueAsFloat(HungerKey, Hunger);
		BlackboardComponent->SetValueAsBool(CanReproduceKey, bCanReproduce);
	}
}

void ACreatureAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!ControlledCreature || !BlackboardComponent)
	{
		return;
	}

	// Find the closest relevant actor
	AActor* ClosestActor = nullptr;
	float ClosestDistance = FLT_MAX;

	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor || Actor == ControlledCreature)
		{
			continue;
		}

		float Distance = FVector::Dist(ControlledCreature->GetActorLocation(), Actor->GetActorLocation());
		
		if (Distance < ClosestDistance)
		{
			// Check if this actor is relevant (food, mate, predator, etc.)
			if (Cast<ACreature>(Actor))
			{
				ClosestActor = Actor;
				ClosestDistance = Distance;
			}
		}
	}

	if (ClosestActor)
	{
		SetTargetActor(ClosestActor);
	}
}

void ACreatureAIController::SetupPerception()
{
	if (!AIPerceptionComponent)
	{
		return;
	}

	// Setup sight sense
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 1600.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}

	// Setup hearing sense
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 800.0f;
		HearingConfig->SetMaxAge(3.0f);
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}

	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}
