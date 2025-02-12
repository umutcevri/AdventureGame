// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "Components/HealthComponent.h"
#include "Weapons/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AddOwnedComponent(HealthComponent);

	MaxStamina = 100.f;
	StaminaRegenRate = 10.f;
	Poise = 100.f;

	MoveState = EMoveState::Running;

	RunSpeed = 500.f;

	WalkSpeed = 200.f;

	bRagdollOnDeath = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AttackState = EAttackState::None;
	CharacterState = ECharacterState::Free;

	Stamina = MaxStamina;

	SpawnDefaultWeapon();
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController->IsA<AAIController>())
	{
		AAIController* AIController = Cast<AAIController>(NewController);
		AIController->SetGenericTeamId(CharacterTeamID);
	}
}

void ACharacterBase::HitReactEnd()
{
	bIsHitReact = false;

	CharacterState = ECharacterState::Free;

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Hit React End"));
}

void ACharacterBase::UpdateWalkSpeed(float Value)
{
	GetCharacterMovement()->MaxWalkSpeed = Value;
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bIsMoving = !GetVelocity().IsNearlyZero();

	if (MoveState == EMoveState::Walking)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	else if (MoveState == EMoveState::Running)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}

	if (bStaminaRegen)
	{
		Stamina += DeltaTime * StaminaRegenRate;
		Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
	}

	if (OnHealthChange.IsBound())
	{
		OnHealthChange.Broadcast(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	}

	if (OnStaminaChange.IsBound())
	{
		OnStaminaChange.Broadcast(Stamina, MaxStamina);
	}

	if (bIsMoving)
	{
		FVector2D CurrentMovementDirection = FVector2D((GetActorTransform().InverseTransformVector(GetVelocity())).GetSafeNormal2D());
		MovementDirection = FMath::Vector2DInterpTo(MovementDirection, CurrentMovementDirection, DeltaTime, 5.f);

		//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Movement Direction: %s"), *MovementDirection.ToString()));
	}

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::GetHit(FHitInfo HitInfo)
{
	if (bIsInvincible)
	{
		return;
	}

	HealthComponent->ChangeHealth(-HitInfo.Damage);

	if (HitInfo.PoiseDamage >= Poise)
	{
		//if ai controller, stop movement
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}

		if (HitInfo.HitRootMotionTranslation != FVector::Zero())
		{
			FVector RootMotionDirection = (GetActorLocation() - HitInfo.HitActorLocation).GetSafeNormal2D();

			TSharedPtr<FRootMotionSource_MoveToForce> RootMotionSource = MakeShared<FRootMotionSource_MoveToForce>();
			RootMotionSource->InstanceName = TEXT("ApplyRemainingRootMotion");
			RootMotionSource->AccumulateMode = ERootMotionAccumulateMode::Override;
			RootMotionSource->Priority = 1;
			RootMotionSource->Duration = 0.6f;
			RootMotionSource->StartLocation = GetActorLocation();
			RootMotionSource->TargetLocation = GetActorLocation() + RootMotionDirection * HitInfo.HitRootMotionTranslation.Y;
			RootMotionSource->bRestrictSpeedToExpected = true;

			GetCharacterMovement()->ApplyRootMotionSource(RootMotionSource);
		}


		FVector HitDirectionRaw = (HitInfo.HitPoint - GetActorLocation()).GetSafeNormal();

		FVector ForwardVector = GetActorForwardVector();
		FVector RightVector = GetActorRightVector();

		float ForwardDot = FVector::DotProduct(HitDirectionRaw, ForwardVector);
		float RightDot = FVector::DotProduct(HitDirectionRaw, RightVector);

		HitDirection = FVector2D(ForwardDot, RightDot);

		HitDirection.Normalize();

		bIsHitReact = true;

		CharacterState = ECharacterState::Stunned;

		//play montage according to hit direction

		if (HitDirection.X >= 0.f && (HitDirection.X >= FMath::Abs(HitDirection.Y)))
		{
			if (HitReactFront)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					EndAttack();
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
					EndAttack();
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
					EndAttack();
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
					EndAttack();
					AnimInstance->Montage_Play(HitReactLeft, 1.0f);
				}
			}
		}

		ChangeCurrentMontageRate(0.1f, 0.2f);
	}

	FRotator ParticleRotation = HitInfo.HitNormal.ToOrientationRotator();

	ParticleRotation = FRotationMatrix::MakeFromZ(HitInfo.HitPoint - GetActorLocation()).Rotator();

	if (BloodParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticle, HitInfo.HitPoint, ParticleRotation, FVector(2.f));
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitInfo.HitPoint);
	}

}

void ACharacterBase::HitOther()
{
	if (!bHitStop)
	{
		HitStop();
	}
}

void ACharacterBase::UpdateStamina(float Value)
{
	Stamina = FMath::Clamp(Stamina + Value, 0.f, MaxStamina);
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
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RightHandSocket");
		}
	}
}

void ACharacterBase::HitStop()
{
	bHitStop = true;

	ChangeCurrentMontageRate(0.1f, 0.2f);
	
}

void ACharacterBase::ChangeCurrentMontageRate(float NewRate, float Duration)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();

		if (CurrentMontage)
		{

			GetMesh()->GetAnimInstance()->Montage_SetPlayRate(CurrentMontage, NewRate);

			GetWorld()->GetTimerManager().SetTimer(
				MontageRateTimerHandle,
				[this, CurrentMontage, AnimInstance]()
				{

					if (CurrentMontage)
					{
						AnimInstance->Montage_SetPlayRate(CurrentMontage, 1.0f);
					}

					bHitStop = false;
				},
				Duration,
				false
			);
		}
	}
}

bool ACharacterBase::IsStaminaEnoughForChosenAttack()
{
	if (ChosenAttack == EAttackState::Primary)
	{
		return Stamina >= PrimaryAttackData.StaminaCost;
	}
	else if (ChosenAttack == EAttackState::Secondary)
	{
		return Stamina >= SecondaryAttackData.StaminaCost;
	}
	else
	{
		return false;
	}
}

void ACharacterBase::ExecuteChosenAttack()
{
	if (ChosenAttack == EAttackState::Primary)
	{
		PrimaryAttack();
	}
	else if (ChosenAttack == EAttackState::Secondary)
	{
		SecondaryAttack();
	}
}

FAttackData ACharacterBase::GetCurrentAttackData()
{
	if (AttackState == EAttackState::Primary)
	{
		return PrimaryAttackData;
	}
	else if (AttackState == EAttackState::Secondary)
	{
		return SecondaryAttackData;
	}
	else
	{
		return FAttackData();
	}
}

FAttackData ACharacterBase::GetChosenAttackData()
{
	if (ChosenAttack == EAttackState::Primary)
	{
		return PrimaryAttackData;
	}
	else if (ChosenAttack == EAttackState::Secondary)
	{
		return SecondaryAttackData;
	}
	else
	{
		return FAttackData();
	}
}

void ACharacterBase::KillCharacter()
{
	CharacterTeamID = FGenericTeamId::NoTeam;

	OnCharacterKilled.Broadcast();

	if (Cast<AAIController>(Controller))
	{
		AController* CurrController = Controller;
		Controller->UnPossess();
		CurrController->Destroy();
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.25f);
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (bRagdollOnDeath)
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

		GetMesh()->SetSimulatePhysics(true);
	}
}

bool ACharacterBase::PrimaryAttack()
{
	
	if (CharacterState == ECharacterState::Free && Stamina >= PrimaryAttackData.StaminaCost && CurrentWeapon && PrimaryAttackData.Montages.Num() > 0)
	{
		if (AttackState != EAttackState::Primary)
		{
			AttackIndex = 0;
		}

		MaxAttackIndex = PrimaryAttackData.Montages.Num() - 1;

		if (AttackIndex > MaxAttackIndex)
		{
			AttackIndex = 0;
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && PrimaryAttackData.Montages[AttackIndex])
		{
			Stamina -= PrimaryAttackData.StaminaCost;

			Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);

			bStaminaRegen = false;

			AnimInstance->Montage_Play(PrimaryAttackData.Montages[AttackIndex], PrimaryAttackData.MontageRate);

			AttackState = EAttackState::Primary;

			CharacterState = ECharacterState::Attacking;

			return true;
		}

	}

	return false;
}

bool ACharacterBase::SecondaryAttack()
{

	if (CharacterState == ECharacterState::Free && Stamina >= SecondaryAttackData.StaminaCost && CurrentWeapon && SecondaryAttackData.Montages.Num() > 0)
	{
		if (AttackState != EAttackState::Secondary)
		{
			AttackIndex = 0;
		}

		MaxAttackIndex = SecondaryAttackData.Montages.Num() - 1;

		if (AttackIndex > MaxAttackIndex)
		{
			AttackIndex = 0;
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && SecondaryAttackData.Montages[AttackIndex])
		{
			Stamina -= SecondaryAttackData.StaminaCost;

			Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);

			bStaminaRegen = false;

			AnimInstance->Montage_Play(SecondaryAttackData.Montages[AttackIndex], SecondaryAttackData.MontageRate);

			AttackState = EAttackState::Secondary;

			CharacterState = ECharacterState::Attacking;

			return true;
		}
	}

	return false;
}

void ACharacterBase::Dodge()
{
	Stamina -= 30.f;
	Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);

	bStaminaRegen = false;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(DodgeMontage, DodgeMontageRate);
	}
}

void ACharacterBase::SaveAttack()
{
	AttackIndex += 1;
	ChosenAttack = EAttackState::None;
	CharacterState = ECharacterState::Free;

	bStaminaRegen = true;
}

void ACharacterBase::EndAttack()
{
	bStaminaRegen = true;
	
	if (CurrentWeapon)
	{
		CurrentWeapon->DisableDamage();
	}
}

void ACharacterBase::EnableInvincibility()
{
	bIsInvincible = true;
}

void ACharacterBase::DisableInvincibility()
{
	bStaminaRegen = true;
	bIsInvincible = false;
	CharacterState = ECharacterState::Free;
}

void ACharacterBase::AIChooseRandomAttack()
{
	if (ChosenAttack != EAttackState::None)
	{
		return;
	}

	uint8 RandomIndex = FMath::RandRange(1, 2);

	ChosenAttack = static_cast<EAttackState>(RandomIndex);
}

