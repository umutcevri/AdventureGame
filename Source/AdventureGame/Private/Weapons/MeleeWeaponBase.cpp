// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MeleeWeaponBase.h"
#include "Components/CapsuleComponent.h"

AMeleeWeaponBase::AMeleeWeaponBase()
{
	WeaponCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollider"));
	WeaponCollider->SetCollisionProfileName("NoCollision");
	WeaponCollider->SetupAttachment(WeaponMesh);
}

void AMeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPerformSweep)
	{
		PerformWeaponSweep();
	}
}

void AMeleeWeaponBase::EnableDamage()
{
	Super::EnableDamage();

	PreviousColliderLocation = WeaponCollider->GetComponentLocation();

	bPerformSweep = true;
}

void AMeleeWeaponBase::DisableDamage()
{
	Super::DisableDamage();

	bPerformSweep = false;
}

void AMeleeWeaponBase::PerformWeaponSweep()
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		WeaponCollider->GetComponentLocation(),
		PreviousColliderLocation,
		WeaponCollider->GetComponentRotation().Quaternion(),
		ECC_GameTraceChannel2,
		FCollisionShape::MakeCapsule(WeaponCollider->GetScaledCapsuleRadius(), WeaponCollider->GetScaledCapsuleHalfHeight()),
		Params
	);

	FColor TraceColor = bHit ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), WeaponCollider->GetComponentLocation(), WeaponCollider->GetScaledCapsuleHalfHeight(), WeaponCollider->GetScaledCapsuleRadius(), WeaponCollider->GetComponentRotation().Quaternion(), TraceColor, false, 1.0f, 0, 2.0f);

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && !HitActors.Contains(HitActor))
		{
			HitActors.Add(HitActor);

			if (ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitActor))
			{
				if (OwnerCharacter->GetGenericTeamId() == HitCharacter->GetGenericTeamId())
				{
					continue;
				}

				OwnerCharacter->HitOther();

				FAttackData AttackData = OwnerCharacter->GetCurrentAttackData();

				FHitInfo HitInfo;
				HitInfo.HitPoint = HitResult.ImpactPoint;
				HitInfo.HitNormal = HitResult.ImpactNormal;
				HitInfo.Damage = AttackData.Damage;
				HitInfo.PoiseDamage = AttackData.PoiseDamage;

				HitInfo.HitActorLocation = OwnerCharacter->GetActorLocation();

				if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
				{
					if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
					{
						float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentMontage);
						float MontageLength = CurrentMontage->GetPlayLength();
						float RemainingTime = MontageLength - CurrentPosition;

						FTransform ExtractedRootMotion = CurrentMontage->ExtractRootMotionFromTrackRange(CurrentPosition, MontageLength);

						FVector RootMotionTranslation = ExtractedRootMotion.GetLocation();

						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Root Motion Translation: %s"), *RootMotionTranslation.ToString()));

						HitInfo.HitRootMotionTranslation = RootMotionTranslation;
						HitInfo.HitRootMotionDuration = RemainingTime;
					}

				}

				HitCharacter->GetHit(HitInfo);

			}
		}
	}
}
