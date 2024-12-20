// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "CharacterBase.generated.h"

USTRUCT()
struct FHitInfo
{
	GENERATED_BODY()

	float Damage;
	FVector HitPoint;
	FVector HitNormal;
};

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Free,
	Mounted,
	Attacking,
};

UCLASS()
class ADVENTUREGAME_API ACharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	
	ACharacterBase();

	ACharacterBase(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly)
	bool bIsClimbing;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MovementDirection;

	UPROPERTY(BlueprintReadOnly)
	FVector2D HitDirection;

	UPROPERTY(BlueprintReadOnly)
	bool bIsJumping;

	UPROPERTY(BlueprintReadOnly)
	EPlayerState _PlayerState;

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
	FGenericTeamId TeamID = 1;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetHit(FHitInfo HitInfo);

	void HitOther();

	FGenericTeamId GetGenericTeamId() const override { return TeamID; }

private:
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

	bool bHitStop = false;

	void HitStop();

	void ChangeCurrentMontageRate(float NewRate, float Duration);

	FTimerHandle MontageRateTimerHandle;

	//hit particle
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UParticleSystem> BloodParticle;

	

};
