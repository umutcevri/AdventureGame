// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class ADVENTUREGAME_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void EnableDamage();

	UFUNCTION(BlueprintCallable)
	void DisableDamage();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool bCanDamage;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	void PerformWeaponTrace();

	FTimerHandle WeaponTraceTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float WeaponTraceInterval = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float TraceSphereRadius = 20.f;

	TArray<AActor*> HitActors;

	TObjectPtr<class ACharacterBase> OwnerCharacter;

};
