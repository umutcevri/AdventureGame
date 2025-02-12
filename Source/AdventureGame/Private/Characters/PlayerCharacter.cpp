// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Characters/Horse.h"
#include "Weapons/WeaponBase.h"
#include "Engine/OverlapResult.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	TPSCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	TPSCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TPSCameraComponent->bUsePawnControlRotation = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 1);
		}
	}
}

void APlayerCharacter::CompleteMounting()
{
	if (CurrentHorse)
	{
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

		AttachToComponent(CurrentHorse->GetMesh(), AttachmentRules, "MountSocket");
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bMoveInput = false;

	if (bDodgeRotation)
	{
		DodgeRotation(DeltaTime);
	}

	UpdateCameraRotation(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PrimaryAttackInput);
		EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::SecondaryAttackInput);
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &APlayerCharacter::LockOn);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &APlayerCharacter::Walk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndWalk);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::DodgeInput);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (CharacterState != ECharacterState::Free)
	{
		return;
	}
	GetMesh()->GetAnimInstance()->StopAllMontages(0.25f);

	bMoveInput = true;

	MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	FVector ForwardDirection;

	FVector RightDirection;

	ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	MovementInput = FVector2D((ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X).GetSafeNormal2D());
	
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
	//do line trace and get the name of overlapped component not actor
	FHitResult HitResult;
	const FVector Start = GetActorLocation() + FVector::UpVector * 50.f;
	const FVector End = Start + GetActorForwardVector() * 100.f;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		if (CharacterState != ECharacterState::Mounted)
		{
			if (AHorse* Horse = Cast<AHorse>(HitResult.GetActor()))
			{
				FString HitComponentName = HitResult.Component->GetName();

				if (HitComponentName == "MountColliderRight" || HitComponentName == "MountColliderLeft")
				{
					CurrentHorse = Horse;
					Mount(HitComponentName);
				}
			}
		}	
	}
}

void APlayerCharacter::PrimaryAttackInput(const FInputActionValue& Value)
{
	PrimaryAttack();
}

void APlayerCharacter::SecondaryAttackInput(const FInputActionValue& Value)
{
	SecondaryAttack();
}

void APlayerCharacter::LockOn(const FInputActionValue& Value)
{
	if (bLockOn)
	{
		LockOnTarget = nullptr;
		bLockOn = false;
		
	}
	else
	{
		LockOnTarget = FindNearestEnemy();
		if (LockOnTarget->IsValidLowLevel())
		{
			Cast<ACharacterBase>(LockOnTarget)->OnCharacterKilled.AddUniqueDynamic(this, &APlayerCharacter::OnLockOnTargetDeath);
			bLockOn = true;
		}
	}
}

void APlayerCharacter::Walk(const FInputActionValue& Value)
{
	MoveState = EMoveState::Walking;
}

void APlayerCharacter::EndWalk(const FInputActionValue& Value)
{
	MoveState = EMoveState::Running;
}

void APlayerCharacter::DodgeInput(const FInputActionValue& Value)
{
	if (CharacterState == ECharacterState::Free)
	{
		CharacterState = ECharacterState::Dodging;

		float TargetYaw = FMath::Atan2(MovementInput.Y, MovementInput.X) * (180.0f / PI);

		DodgeStartRotation = GetActorRotation();
		DodgeTargetRotation = FRotator(0.0f, TargetYaw, 0.0f);

		bDodgeRotation = true;
	}
}

void APlayerCharacter::Mount(FString MountColliderName)
{

	if (MountColliderName == "MountColliderRight" && MountHorseMontageRight)
	{	
		GetMesh()->GetAnimInstance()->Montage_Play(MountHorseMontageRight, 1.0f);
			
		FTransform HorseTransform = CurrentHorse->GetActorTransform();
		FVector MountLocation = HorseTransform.GetLocation() + HorseTransform.TransformVector(CurrentHorse->RightMountLocation);
		FRotator MountRotation = CurrentHorse->GetActorRotation();
		MountRotation.Yaw -= 90.f;
		SetActorLocationAndRotation(MountLocation, MountRotation);	
	}
	else if (MountColliderName == "MountColliderLeft" && MountHorseMontageLeft)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MountHorseMontageLeft, 1.0f);
		FTransform HorseTransform = CurrentHorse->GetActorTransform();
		FVector MountLocation = HorseTransform.GetLocation() + HorseTransform.TransformVector(CurrentHorse->LeftMountLocation);
		FRotator MountRotation = CurrentHorse->GetActorRotation();
		MountRotation.Yaw += 90.f;
		SetActorLocationAndRotation(MountLocation, MountRotation);
	}

	CharacterState = ECharacterState::Mounted;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		PlayerController->Possess(Cast<APawn>(CurrentHorse));
	}

	//disable actor collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

AActor* APlayerCharacter::FindNearestEnemy()
{
	TArray<FOverlapResult> OverlapResults;

	TArray<ACharacterBase*> TargetEnemies;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(this);

	bool bOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(1000.f),
		QueryParams
	);

	if (!bOverlap)
	{
		return nullptr;
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ACharacterBase* OverlappingCharacter = Cast<ACharacterBase>(OverlapResult.GetActor());
		if (OverlappingCharacter && !TargetEnemies.Contains(OverlappingCharacter))
		{
			//print team id
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, FString::Printf(TEXT("Team ID: %d"), OverlappingCharacter->GetGenericTeamId().GetId()));

			if(OverlappingCharacter->GetGenericTeamId() != FGenericTeamId::NoTeam && OverlappingCharacter->GetGenericTeamId() != GetGenericTeamId())
			{
				TargetEnemies.Add(OverlappingCharacter);
			}
		}
	}

	if (TargetEnemies.Num() > 0)
	{
		ACharacterBase* NearestEnemy = TargetEnemies[0];
		for (ACharacterBase* TargetEnemy : TargetEnemies)
		{
			if (FVector::DistSquared(TargetEnemy->GetActorLocation(), GetActorLocation()) < FVector::DistSquared(NearestEnemy->GetActorLocation(), GetActorLocation()))
			{
				NearestEnemy = TargetEnemy;
			}
		}

		return NearestEnemy;
	}

	return nullptr;
}

void APlayerCharacter::UpdateCameraRotation(float DeltaTime)
{
	if (bLockOn && LockOnTarget->IsValidLowLevel())
	{
		//print lock on target name
		//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Lock On Target: %s"), *LockOnTarget->GetName()));

		float Distance = FVector::Dist(LockOnTarget->GetActorLocation(), GetActorLocation());

		if (Distance > LockCutOffDistance)
		{
			bLockOn = false;
			LockOnTarget = nullptr;
			GetCharacterMovement()->bOrientRotationToMovement = true;

			return;
		}

		GetCharacterMovement()->bOrientRotationToMovement = false;

		FVector Direction = LockOnTarget->GetActorLocation() - GetActorLocation();
		FRotator DesiredRotation = FRotationMatrix::MakeFromX(Direction).Rotator();

		//add pitch relative to distance to target
		float Pitch = FMath::Lerp(-45.f, -10.f, Distance / LockCutOffDistance);

		DesiredRotation.Pitch += Pitch;

		// Interpolate smoothly between current and desired rotation
		FRotator NewRotation = FMath::RInterpTo(GetControlRotation(), DesiredRotation, DeltaTime, 10.0f);
		Controller->SetControlRotation(NewRotation);

		if (CharacterState != ECharacterState::Dodging)
		{
			FRotator TargetCharacterRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);
			FRotator CharacterRotation = FMath::RInterpTo(GetActorRotation(), TargetCharacterRotation, DeltaTime, 10.0f);
			SetActorRotation(CharacterRotation);
		}
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void APlayerCharacter::OnLockOnTargetDeath()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Lock On Target Died"));
	LockOnTarget = FindNearestEnemy();
	if (!LockOnTarget->IsValidLowLevel())
	{
		bLockOn = false;
	}
	else
	{
		Cast<ACharacterBase>(LockOnTarget)->OnCharacterKilled.AddUniqueDynamic(this, &APlayerCharacter::OnLockOnTargetDeath);
	}
}

void APlayerCharacter::DodgeRotation(float DeltaTime)
{
	DodgeRotationTimer += DeltaTime;

	float Alpha = FMath::Clamp(DodgeRotationTimer / DodgeRotationDuration, 0.0f, 1.0f);

	FRotator NewRotation = FMath::Lerp(DodgeStartRotation, DodgeTargetRotation, Alpha);

	SetActorRotation(NewRotation);

	if (DodgeRotationTimer >= DodgeRotationDuration)
	{
		SetActorRotation(DodgeTargetRotation);

		bDodgeRotation = false;

		DodgeRotationTimer = 0.f;

		Dodge();
	}
}






