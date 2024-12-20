// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "Components/HealthComponent.h"
#include "Weapons/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AddOwnedComponent(HealthComponent);
}

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AddOwnedComponent(HealthComponent);

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	
}

void ACharacterBase::HitReactEnd()
{
	bIsHitReact = false;

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Hit React End"));
}

void ACharacterBase::UpdateWalkSpeed(float Value)
{
	GetCharacterMovement()->MaxWalkSpeed = Value;
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::GetHit(FHitInfo HitInfo)
{
	FVector HitDirectionRaw = (HitInfo.HitPoint - GetActorLocation()).GetSafeNormal();

	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();    

	float ForwardDot = FVector::DotProduct(HitDirectionRaw, ForwardVector);
	float RightDot = FVector::DotProduct(HitDirectionRaw, RightVector);

	HitDirection = FVector2D(ForwardDot, RightDot);

	HitDirection.Normalize();

	bIsHitReact = true;

	//play montage according to hit direction

	if (HitDirection.X >= 0.f && (HitDirection.X >= FMath::Abs(HitDirection.Y)))
	{
		if (HitReactFront)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(HitReactFront, 1.0f);
			}
		}
	}
	else if (HitDirection.X < 0.f && (FMath::Abs(HitDirection.X) >= FMath::Abs(HitDirection.Y)))
	{
		if (HitReactBack)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(HitReactBack, 1.0f);
			}
		}
	}
	else if (HitDirection.Y >= 0.f)
	{
		if (HitReactRight)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(HitReactRight, 1.0f);
			}
		}
	}
	else if (HitDirection.Y < 0.f)
	{
		if (HitReactLeft)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(HitReactLeft, 1.0f);
			}
		}
	}

	ChangeCurrentMontageRate(0.1f, 0.2f);

	FRotator ParticleRotation = HitInfo.HitNormal.ToOrientationRotator();

	ParticleRotation = FRotationMatrix::MakeFromZ(HitInfo.HitPoint - GetActorLocation()).Rotator();

	if (BloodParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticle, HitInfo.HitPoint, ParticleRotation, FVector(1.f));
	}

}

void ACharacterBase::HitOther()
{
	if (!bHitStop)
	{
		HitStop();
	}
}

void ACharacterBase::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActorDeferred<AWeaponBase>(DefaultWeaponClass, FTransform::Identity, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		GetCapsuleComponent()->IgnoreActorWhenMoving(CurrentWeapon, true);

		CurrentWeapon->FinishSpawning(FTransform::Identity);

		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "RightHandSocket");
		}
	}
}

void ACharacterBase::HitStop()
{
	bHitStop = true;

	ChangeCurrentMontageRate(0.1f, 0.1f);
	
}

void ACharacterBase::ChangeCurrentMontageRate(float NewRate, float Duration)
{
	UAnimMontage* CurrentMontage = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();

	if (CurrentMontage)
	{
		
		GetMesh()->GetAnimInstance()->Montage_SetPlayRate(CurrentMontage, NewRate);

		GetWorld()->GetTimerManager().SetTimer(
			MontageRateTimerHandle,
			[this, CurrentMontage]()
			{
				
				if (CurrentMontage)
				{
					GetMesh()->GetAnimInstance()->Montage_SetPlayRate(CurrentMontage, 1.0f);
				}

				bHitStop = false;
			},
			Duration, 
			false     
		);
	}
}

