// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Components/BoxComponent.h"
#include "Components/HealthComponent.h"
#include "Characters/CharacterBase.h"
#include "Characters/PlayerCharacter.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());

	bCanDamage = true;
}

void AWeaponBase::EnableDamage()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Enabled Damage"));

	if (!GetWorld()->GetTimerManager().IsTimerActive(WeaponTraceTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(WeaponTraceTimerHandle, this, &AWeaponBase::PerformWeaponTrace, WeaponTraceInterval, true);
	}

	HitActors.Empty();
}

void AWeaponBase::DisableDamage()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Disabled Damage"));

	if (APlayerCharacter* OwnerPlayer = Cast<APlayerCharacter>(OwnerCharacter))
	{
		OwnerPlayer->bAttackRotation = false;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(WeaponTraceTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(WeaponTraceTimerHandle);
	}
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::PerformWeaponTrace()
{
	// Define start and end points of the trace
	FVector StartPoint = WeaponMesh->GetSocketLocation("WeaponStart");
	FVector EndPoint = WeaponMesh->GetSocketLocation("WeaponEnd");

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	// Perform the sphere sweep multi
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartPoint,
		EndPoint,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(TraceSphereRadius),
		Params
	);

	/*
	FColor TraceColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugSphere(GetWorld(), StartPoint, TraceSphereRadius, 12, TraceColor, false, 1.0f);
	DrawDebugSphere(GetWorld(), EndPoint, TraceSphereRadius, 12, TraceColor, false, 1.0f);
	DrawDebugLine(GetWorld(), StartPoint, EndPoint, TraceColor, false, 1.0f, 0, 2.0f);
	*/

	//check if hit actor is in hitactors array

	for (const FHitResult &HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && !HitActors.Contains(HitActor))
		{

			OwnerCharacter->HitOther();

			HitActors.Add(HitActor);
			if (ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitActor))
			{
				FHitInfo HitInfo;
				HitInfo.HitPoint = HitResult.ImpactPoint;
				HitInfo.HitNormal = HitResult.ImpactNormal;
				HitInfo.Damage = 10.f;

				HitCharacter->GetHit(HitInfo);
			}
		}
	}
	

}

