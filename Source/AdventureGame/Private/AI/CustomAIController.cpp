// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CustomAIController.h"
#include "Characters/AICharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

ACustomAIController::ACustomAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	AIPerceptionComponent->ConfigureSense(*SightConfig);
}

void ACustomAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (ControlledCharacter = Cast<AAICharacter>(InPawn))
	{
		if (UBehaviorTree* BehaviorTree = ControlledCharacter->GetBehaviorTree())
		{
			RunBehaviorTree(BehaviorTree);
		}

	}
}

void ACustomAIController::BeginPlay()
{
	Super::BeginPlay();

	GetBlackboardComponent()->SetValueAsVector("PatrolLocation", GetPawn()->GetActorLocation());

	FGenericTeamId::SetAttitudeSolver([](const FGenericTeamId& TeamA, const FGenericTeamId& TeamB)
		{
			if (TeamB == FGenericTeamId::NoTeam)
			{
				return ETeamAttitude::Neutral;
			}
			else if (TeamA == TeamB)
			{
				return ETeamAttitude::Friendly;
			}
			else
			{
				return ETeamAttitude::Hostile;
			}
		});

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACustomAIController::OnTargetPerceptionUpdated);


}

void ACustomAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetBlackboardComponent()->SetValueAsObject("EnemyActor", SensedActor.Get());

	if (ControlledCharacter)
	{
		GetBlackboardComponent()->SetValueAsEnum("CharacterState", (uint8)ControlledCharacter->GetCharacterState());
	}
}

void ACustomAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (ACharacterBase* SensedCharacter = Cast<ACharacterBase>(Actor))
	{
		if (FGenericTeamId::GetAttitude(GetGenericTeamId(), SensedCharacter->GetGenericTeamId()) == ETeamAttitude::Hostile)
		{
			if (Stimulus.WasSuccessfullySensed())
			{

				if (!SensedActor.IsValid())
				{
					GetWorld()->GetTimerManager().ClearTimer(EnemyLostTimerHandle);

					SensedActor = Actor;

					//print name of sensed actor
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, FString::Printf(TEXT("Sensed Actor: %s"), *SensedActor->GetName()));

					SensedCharacter->OnCharacterKilled.AddUniqueDynamic(this, &ACustomAIController::OnEnemyDeath);
					
				}
				else if (Actor == SensedActor)
				{
					GetWorld()->GetTimerManager().ClearTimer(EnemyLostTimerHandle);
				}

			}
			else
			{
				if (Actor == SensedActor)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemy Lost"));
					GetWorld()->GetTimerManager().SetTimer(EnemyLostTimerHandle, this, &ACustomAIController::OnEnemyLost, EnemyLostTime, false);
				}
			}
		}
	}
}

void ACustomAIController::OnEnemyLost()
{

	if (SensedActor.IsValid())
	{
		if (ACharacterBase* SensedCharacter = Cast<ACharacterBase>(SensedActor))
		{
			SensedCharacter->OnCharacterKilled.RemoveDynamic(this, &ACustomAIController::OnEnemyDeath);
		}

		AIPerceptionComponent->ForgetActor(SensedActor.Get());
	}
	
	SensedActor = GetFirstPerceivedEnemy();

	if (ACharacterBase* SensedCharacter = Cast<ACharacterBase>(SensedActor))
	{
		SensedCharacter->OnCharacterKilled.AddUniqueDynamic(this, &ACustomAIController::OnEnemyDeath);
	}
}

void ACustomAIController::OnEnemyDeath()
{
	GetWorld()->GetTimerManager().ClearTimer(EnemyLostTimerHandle);

	if (SensedActor.IsValid())
	{
		AIPerceptionComponent->ForgetActor(SensedActor.Get());
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemy Killed"));

	SensedActor = GetFirstPerceivedEnemy();

	//print name of sensed actor
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, FString::Printf(TEXT("Sensed Actor: %s"), *SensedActor->GetName()));

	if (ACharacterBase* SensedCharacter = Cast<ACharacterBase>(SensedActor))
	{
		SensedCharacter->OnCharacterKilled.AddUniqueDynamic(this, &ACustomAIController::OnEnemyDeath);
	}
}

void ACustomAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (RequestID.IsEquivalent(BackOffRequestID))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Back Off Completed"));
		bBackingOff = false;
	}
}

AActor* ACustomAIController::GetFirstPerceivedEnemy()
{
	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetPerceivedHostileActors(PerceivedActors);

	//loop through perceived actors and return the first one with team attitude hostile
	for (AActor* Actor : PerceivedActors)
	{
		if (ACharacterBase* PerceivedCharacter = Cast<ACharacterBase>(Actor))
		{
			if (FGenericTeamId::GetAttitude(GetGenericTeamId(), PerceivedCharacter->GetGenericTeamId()) == ETeamAttitude::Hostile)
			{
				//print team id of perceived character
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, FString::Printf(TEXT("Perceived Character Team ID: %d"), PerceivedCharacter->GetGenericTeamId().GetId()));
				return Actor;
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Perceived Enemy"));

	return nullptr;
}
