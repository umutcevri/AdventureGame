// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "MeleeWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API AMeleeWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	AMeleeWeaponBase();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void EnableDamage() override;

	virtual void DisableDamage() override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<class UCapsuleComponent> WeaponCollider;

	void PerformWeaponSweep();

	FVector PreviousColliderLocation;

	bool bPerformSweep = false;
	
};
