// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterBase.generated.h"

USTRUCT()
struct FHitInfo
{
	GENERATED_BODY()

	float Damage;
	float PoiseDamage;
	FVector HitPoint;
	FVector HitNormal;
	FVector HitActorLocation;
	FVector HitRootMotionTranslation = FVector::Zero();
	float HitRootMotionDuration;
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Free,
	Mounted,
	Attacking,
	Stunned,
	Dodging,
};

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	None,
	Primary,
	Secondary,
};

UENUM(BlueprintType)
enum class EMoveState : uint8
{
	Walking,
	Running,
};

USTRUCT()
struct FAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float StaminaCost = 30.f;

	UPROPERTY(EditAnywhere)
	float Damage = 30.f;

	UPROPERTY(EditAnywhere)
	float PoiseDamage = 30.f;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditAnywhere)
	float MontageRate = 1.f;

	UPROPERTY(EditAnywhere)
	float AIAttackDistance = 100.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChange, float, NewHealth, float, MaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChange, float, NewStamina, float, MaxStamina);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterKilled);

UCLASS()
class ADVENTUREGAME_API ACharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MovementDirection;

	UPROPERTY(BlueprintReadOnly)
	FVector2D HitDirection;

	UPROPERTY(BlueprintReadOnly)
	ECharacterState CharacterState;

	EMoveState MoveState;

	UPROPERTY(BlueprintReadOnly)
	bool bIsHitReact = false;

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class AWeaponBase> DefaultWeaponClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AWeaponBase> CurrentWeapon;

	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(float Value);

	UPROPERTY(EditAnywhere, Category = "AI")
	FGenericTeamId CharacterTeamID = 1;

	bool PrimaryAttack();

	bool SecondaryAttack();

	void Dodge();

	UFUNCTION(BlueprintCallable)
	void SaveAttack();

	UFUNCTION(BlueprintCallable)
	void EndAttack();

	UFUNCTION(BlueprintCallable)
	void EnableInvincibility();

	UFUNCTION(BlueprintCallable)
	void DisableInvincibility();

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AttackTarget;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetHit(FHitInfo HitInfo);

	void HitOther();

	UFUNCTION(BlueprintCallable)
	void AIChooseRandomAttack();

	UPROPERTY(BlueprintReadOnly)
	EAttackState ChosenAttack;

	UFUNCTION(BlueprintCallable)
	void ExecuteChosenAttack();

	FAttackData GetCurrentAttackData();

	FAttackData GetChosenAttackData();

	virtual void KillCharacter();

	UFUNCTION(BlueprintCallable)
	bool IsStaminaEnoughForChosenAttack();

	void SetAttackTarget(AActor* NewTarget) { AttackTarget = NewTarget; }

	FGenericTeamId GetGenericTeamId() const override { return CharacterTeamID; }

	ECharacterState GetCharacterState() const { return CharacterState; }

	EMoveState GetMoveState() const { return MoveState; }

	bool IsMoving() const { return bIsMoving; }

	FVector2D GetMovementDirection() const { return MovementDirection; }

	bool IsFalling() const { return GetCharacterMovement()->IsFalling(); }

	void SetMoveState(EMoveState NewMoveState) { MoveState = NewMoveState; }

	UPROPERTY()
	FOnCharacterKilled OnCharacterKilled;

private:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float RunSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float Poise;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float StaminaRegenRate;



	void UpdateStamina(float Value);

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UHealthComponent> HealthComponent;

	void SpawnDefaultWeapon();

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactFront;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactBack;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactLeft;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactRight;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float DodgeMontageRate = 1.0f;

	bool bHitStop = false;

	bool bIsInvincible = false;

	void HitStop();

	void ChangeCurrentMontageRate(float NewRate, float Duration);

	FTimerHandle MontageRateTimerHandle;

	//hit particle
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UParticleSystem> BloodParticle;

	int AttackIndex;

	int MaxAttackIndex;
	EAttackState AttackState;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FAttackData PrimaryAttackData;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FAttackData SecondaryAttackData;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnHealthChange OnHealthChange;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStaminaChange OnStaminaChange;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* HitSound;

	bool bStaminaRegen = true;

	UPROPERTY(EditAnywhere, Category = "Character")
	bool bRagdollOnDeath;

};
