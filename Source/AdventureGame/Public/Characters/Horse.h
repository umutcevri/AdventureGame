// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Horse.generated.h"

UENUM(BlueprintType)
enum class EHorseState : uint8
{
	Idle,
	Walk,
	Gallop,
};

UCLASS()
class ADVENTUREGAME_API AHorse : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHorse();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(BlueprintReadOnly)
	EHorseState CurrentState;

	UPROPERTY(BlueprintReadOnly)
	float AnimMovementDirection;

	UPROPERTY(BlueprintReadOnly)
	float AnimPlayRate;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Mounting")
	FVector LeftMountLocation;

	UPROPERTY(EditAnywhere, Category = "Mounting")
	FVector RightMountLocation;

private:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> TPSCameraComponent;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	void Look(const FInputActionValue& Value);

	void LookEnd(const FInputActionValue& Value);

	void Move(const FInputActionValue& Value);

	void MoveEnd(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GallopSpeed;

	float MovementDirection;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> BreakMontage;

	void Walk(const FInputActionValue& Value);

	void WalkEnd(const FInputActionValue& Value);

	bool bIsWalking;

	bool bMoveInput;

	bool bLookInput;

	float PreviousYaw;

	FVector PreviousVelocity;

	UPROPERTY(EditAnywhere, Category = "Player")
	TObjectPtr<class USphereComponent> MountColliderRight;

	UPROPERTY(EditAnywhere, Category = "Player")
	TObjectPtr<class USphereComponent> MountColliderLeft;

};
