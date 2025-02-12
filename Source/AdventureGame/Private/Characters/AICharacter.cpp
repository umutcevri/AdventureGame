// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AICharacter.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"

AAICharacter::AAICharacter()
{
	StatsWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatsWidget"));
	StatsWidget->SetupAttachment(RootComponent);
}

void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FocusActor.IsValid())
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), FocusActor->GetActorLocation());
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, 10.f);
		SetActorRotation(NewRotation);
	}
}

void AAICharacter::KillCharacter()
{
	Super::KillCharacter();

	if (StatsWidget)
	{
		StatsWidget->SetVisibility(false);
	}
}
