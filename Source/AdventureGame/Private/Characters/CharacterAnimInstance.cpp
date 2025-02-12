// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterAnimInstance.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacterBase>(GetOwningActor());
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		return;
	}

	IsMoving = OwnerCharacter->IsMoving();
	IsFalling = OwnerCharacter->IsFalling();
	MovementDirection = OwnerCharacter->GetMovementDirection();
	CharacterState = OwnerCharacter->GetCharacterState();
	MoveState = OwnerCharacter->GetMoveState();
}
