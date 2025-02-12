// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_Patrol.h"
#include "AI/CustomAIController.h"
#include "Characters/AICharacter.h"

UBTTask_Patrol::UBTTask_Patrol()
{
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ACustomAIController* Controller = Cast<ACustomAIController>(OwnerComp.GetAIOwner()))
	{
		if (AAICharacter* ControlledCharacter = Cast<AAICharacter>(Controller->GetPawn()))
		{
			ControlledCharacter->SetMoveState(EMoveState::Walking);
			//Controller->ClearFocus(EAIFocusPriority::Gameplay);
			ControlledCharacter->SetFocusActor(nullptr);
			ControlledCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
	}

	return EBTNodeResult::Succeeded;
}
