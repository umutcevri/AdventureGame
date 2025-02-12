// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_EngageEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API UBTTask_EngageEnemy : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EngageEnemy();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
