// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AncientGolem.h"

// Sets default values
AAncientGolem::AAncientGolem()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAncientGolem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAncientGolem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAncientGolem::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

