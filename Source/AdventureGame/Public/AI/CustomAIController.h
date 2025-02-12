// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CustomAIController.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API ACustomAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACustomAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

	bool bBackingOff = false;

	FAIRequestID BackOffRequestID;

private:
    UPROPERTY(EditAnywhere, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Sight Configuration
    UPROPERTY(EditAnywhere, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TWeakObjectPtr<AActor> SensedActor;

	TObjectPtr<class AAICharacter> ControlledCharacter;

    UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	FTimerHandle EnemyLostTimerHandle;

	UPROPERTY(EditAnywhere, Category = "AI")
	float EnemyLostTime = 3.f;

	UFUNCTION()
	void OnEnemyLost();

	UFUNCTION()
	void OnEnemyDeath();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	AActor* GetFirstPerceivedEnemy();
};
