// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AICharacter.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API AAICharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AAICharacter();

	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	void SetFocusActor(AActor* Actor) { FocusActor = Actor; }

	virtual void Tick(float DeltaTime) override;

	virtual void KillCharacter() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> StatsWidget;

private:
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	TWeakObjectPtr<AActor> FocusActor;
};
