// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.0f;
	Health = MaxHealth;
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::ChangeHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);

	if (Health == 0.0f)
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->Destroy();
		}
	}

}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

