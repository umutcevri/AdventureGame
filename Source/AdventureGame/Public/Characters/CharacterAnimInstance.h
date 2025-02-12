// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterBase.h"
#include "CharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsMoving;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterState CharacterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMoveState MoveState;

private:
	ACharacterBase* OwnerCharacter;
	
	
};
