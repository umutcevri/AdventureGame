// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CustomCharacterMovementComponent.h"
#include "PlayerCharacter.generated.h"

enum class EAttackState : uint8
{
	None,
	Primary,
	Secondary,
};

UCLASS()
class ADVENTUREGAME_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
	UCustomCharacterMovementComponent* MovementComponent;

	

	UFUNCTION(BlueprintCallable)
	void CompleteMounting();

	UFUNCTION(BlueprintCallable)
	void SaveAttack();

	UFUNCTION(BlueprintCallable)
	void EndAttack();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE UCustomCharacterMovementComponent* GetCustomCharacterMovement() const { return MovementComponent; }

	bool bAttackRotation = false;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ClimbAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ReleaseAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PrimaryAttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SecondaryAttackAction;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> TPSCameraComponent;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Climb(const FInputActionValue& Value);

	void Release(const FInputActionValue& Value);

	void InitJump(const FInputActionValue& Value);

	void Interact(const FInputActionValue& Value);

	void PrimaryAttack(const FInputActionValue& Value);

	void SecondaryAttack(const FInputActionValue& Value);

	FVector2D MovementVector;

	bool bMoveInput;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> JumpMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> MountHorseMontageRight;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> MountHorseMontageLeft;

	TObjectPtr<class AHorse> CurrentHorse;

	void Mount(FString MountColliderName);

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<TObjectPtr<UAnimMontage>> PrimaryAttackMontages;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<TObjectPtr<UAnimMontage>> SecondaryAttackMontages;

	bool bCanAttack;

	int AttackIndex;

	int MaxAttackIndex;
	EAttackState AttackState;

	AActor* FindNearestEnemy();

	

	FVector AttackLocation;

	void RotateTowardsLocation(const FVector& TargetLocation);

	
};
