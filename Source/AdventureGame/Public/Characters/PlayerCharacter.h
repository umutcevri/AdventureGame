// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.generated.h"


UCLASS()
class ADVENTUREGAME_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable)
	void CompleteMounting();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PrimaryAttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SecondaryAttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> TPSCameraComponent;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact(const FInputActionValue& Value);

	void PrimaryAttackInput(const FInputActionValue& Value);

	void SecondaryAttackInput(const FInputActionValue& Value);

	void LockOn(const FInputActionValue& Value);

	void Walk(const FInputActionValue& Value);

	void EndWalk(const FInputActionValue& Value);

	void DodgeInput(const FInputActionValue& Value);

	FVector2D MovementVector;

	FVector2D MovementInput;

	bool bMoveInput;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> JumpMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> MountHorseMontageRight;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> MountHorseMontageLeft;

	TObjectPtr<class AHorse> CurrentHorse;

	void Mount(FString MountColliderName);

	AActor* FindNearestEnemy();
	TWeakObjectPtr<AActor> LockOnTarget;

	bool bLockOn = false;
	
	void UpdateCameraRotation(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LockCutOffDistance = 1500.f;

	UFUNCTION()
	void OnLockOnTargetDeath();

	bool bDodgeRotation = false;

	void DodgeRotation(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float DodgeRotationDuration = 0.25f;

	float DodgeRotationTimer = 0.f;

	FRotator DodgeStartRotation;

	FRotator DodgeTargetRotation;
};
