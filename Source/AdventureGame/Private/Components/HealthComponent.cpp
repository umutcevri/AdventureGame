// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"
#include "Characters/CharacterBase.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.f;

	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::ChangeHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);

	if (bIsAlive && Health == 0.f)
	{
		bIsAlive = false;
		if (AActor* Owner = GetOwner())
		{
			if (ACharacterBase* OwnerCharacter = Cast<ACharacterBase>(Owner))
			{
				OwnerCharacter->KillCharacter();
			}
			else
			{
				Owner->Destroy();
			}
		}
	}

}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

