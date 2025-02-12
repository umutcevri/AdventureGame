// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HealthComponent.h"
#include "Characters/CharacterBase.h"
#include "Characters/PlayerCharacter.h"
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void EnableDamage();

	UFUNCTION(BlueprintCallable)
	virtual void DisableDamage();

	UFUNCTION(BlueprintCallable)
	void PlayWeaponSound();

	TArray<AActor*> HitActors;

	TObjectPtr<ACharacterBase> OwnerCharacter;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

private:
	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* WeaponSound;
};
