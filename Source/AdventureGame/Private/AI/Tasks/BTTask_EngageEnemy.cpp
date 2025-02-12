// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_EngageEnemy.h"
#include "AI/CustomAIController.h"
#include "Characters/AICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_EngageEnemy::UBTTask_EngageEnemy()
{

}

EBTNodeResult::Type UBTTask_EngageEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ACustomAIController* Controller = Cast<ACustomAIController>(OwnerComp.GetAIOwner()))
	{
		if (AAICharacter* ControlledCharacter = Cast<AAICharacter>(Controller->GetPawn()))
		{
			if (AActor* Enemy = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("EnemyActor")))
			{
				//Controller->SetFocus(Enemy, EAIFocusPriority::Gameplay);
				ControlledCharacter->SetFocusActor(Enemy);
				ControlledCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;

				if (ControlledCharacter->ChosenAttack == EAttackState::None)
				{
					ControlledCharacter->AIChooseRandomAttack();
				}

				if (ControlledCharacter->IsStaminaEnoughForChosenAttack())
				{
					ControlledCharacter->SetMoveState(EMoveState::Running);

					Controller->bBackingOff = false;

					ControlledCharacter->GetMesh()->GetAnimInstance()->StopAllMontages(0.25f);

					EPathFollowingRequestResult::Type MoveResult = Controller->MoveToActor(Enemy, ControlledCharacter->GetChosenAttackData().AIAttackDistance);

					if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
					{
						ControlledCharacter->ExecuteChosenAttack();
					}
				}
				else if (!Controller->bBackingOff)
				{

					if (UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
					{
						FVector Origin = Enemy->GetActorLocation() + ControlledCharacter->GetActorForwardVector() * -400.f + ControlledCharacter->GetActorRightVector() * FMath::FRandRange(-400.f, 400.f);

						FNavLocation TargetLocation;
						if (NavigationSystem->ProjectPointToNavigation(Origin, TargetLocation))
						{
							ControlledCharacter->SetMoveState(EMoveState::Walking);

							ControlledCharacter->GetMesh()->GetAnimInstance()->StopAllMontages(0.25f);

							Controller->MoveToLocation(TargetLocation.Location);
							
							Controller->BackOffRequestID = Controller->GetCurrentMoveRequestID();

							Controller->bBackingOff = true;
						}
					}
				}
			}
		}
	}

	return EBTNodeResult::Succeeded;
}
