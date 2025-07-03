#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindFood.generated.h"

class ACreature;
class ANaniteEnvironment;

UCLASS()
class ECOSYSTEMSANDBOX_API UBTTask_FindFood : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindFood();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SearchRadius = 1000.0f;

private:
	ACreature* GetCreature(UBehaviorTreeComponent& OwnerComp) const;
	ANaniteEnvironment* FindNearestEnvironment(const FVector& Location) const;
};
