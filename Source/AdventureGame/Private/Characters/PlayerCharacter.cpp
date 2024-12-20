// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Characters/Horse.h"
#include "Weapons/WeaponBase.h"
#include "Engine/OverlapResult.h"


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	MovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

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

	_PlayerState = EPlayerState::Free;
	AttackState = EAttackState::None;

	bCanAttack = true;
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

void APlayerCharacter::SaveAttack()
{
	bCanAttack = true;
	AttackIndex += 1;
}

void APlayerCharacter::EndAttack()
{
	bCanAttack = true;
	AttackState = EAttackState::None;
	AttackIndex = 0;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bIsMoving = !GetVelocity().IsNearlyZero();

	bIsClimbing = MovementComponent->IsClimbing();

	if(bMoveInput)
	{
		MovementDirection = FMath::Vector2DInterpTo(MovementDirection, MovementVector, DeltaTime, 5.f);
	}
	else
	{
		MovementDirection = FMath::Vector2DInterpTo(MovementDirection, FVector2D::ZeroVector, DeltaTime, 5.f);
	}

	bMoveInput = false;

	if (GetCharacterMovement()->Velocity.Z <= 0)
	{
		bIsJumping = false;
	}

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Movement Vector: %s"), *MovementDirection.ToString()));

	if (bAttackRotation)
	{
		RotateTowardsLocation(AttackLocation);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Started, this, &APlayerCharacter::Climb);
		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &APlayerCharacter::Release);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::InitJump);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PrimaryAttack);
		EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::SecondaryAttack);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (_PlayerState != EPlayerState::Free)
	{
		return;
	}

	bMoveInput = true;

	MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	FVector ForwardDirection;

	FVector RightDirection;

	if (MovementComponent->IsClimbing())
	{
		ForwardDirection = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), -GetActorRightVector());

		RightDirection = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), GetActorUpVector());
	}
	else
	{
		ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Forward: %s"), *ForwardDirection.ToString()));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Right: %s"), *RightDirection.ToString()));

	//print movement vector
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Movement Vector: %s"), *MovementVector.ToString()));
	
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void APlayerCharacter::Climb(const FInputActionValue& Value)
{
	MovementComponent->Climb();
}

void APlayerCharacter::Release(const FInputActionValue& Value)
{
	MovementComponent->Release();
}

void APlayerCharacter::InitJump(const FInputActionValue& Value)
{
	Jump();
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && JumpMontage)
	{
		AnimInstance->Montage_Play(JumpMontage, 1.0f);
	}

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
		if (_PlayerState != EPlayerState::Mounted)
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

void APlayerCharacter::PrimaryAttack(const FInputActionValue& Value)
{
	AActor* NearestEnemy = FindNearestEnemy();

	if(NearestEnemy)
	{
		bAttackRotation = true;
		AttackLocation = NearestEnemy->GetActorLocation();
	}
	else
	{
		bAttackRotation = false;
	}

	if (bCanAttack && CurrentWeapon && PrimaryAttackMontages.Num() > 0)
	{
		if (AttackState != EAttackState::Primary)
		{
			AttackIndex = 0;
		}

		AttackState = EAttackState::Primary;

		bCanAttack = false;

		MaxAttackIndex = PrimaryAttackMontages.Num() - 1;

		if (AttackIndex > MaxAttackIndex)
		{
			AttackIndex = 0;
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && PrimaryAttackMontages[AttackIndex])
		{
			AnimInstance->Montage_Play(PrimaryAttackMontages[AttackIndex], 1.0f);
		}

		CurrentWeapon->bCanDamage = true;
	}
}

void APlayerCharacter::SecondaryAttack(const FInputActionValue& Value)
{
	AActor* NearestEnemy = FindNearestEnemy();

	if (NearestEnemy)
	{
		bAttackRotation = true;
		AttackLocation = NearestEnemy->GetActorLocation();
	}
	else
	{
		bAttackRotation = false;
	}

	if (bCanAttack && CurrentWeapon && SecondaryAttackMontages.Num() > 0)
	{
		if(AttackState != EAttackState::Secondary)
		{
			AttackIndex = 0;
		}

		AttackState = EAttackState::Secondary;

		bCanAttack = false;

		MaxAttackIndex = SecondaryAttackMontages.Num() - 1;

		if (AttackIndex > MaxAttackIndex)
		{
			AttackIndex = 0;
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && SecondaryAttackMontages[AttackIndex])
		{
			AnimInstance->Montage_Play(SecondaryAttackMontages[AttackIndex], 1.0f);
		}

		CurrentWeapon->bCanDamage = true;
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

	_PlayerState = EPlayerState::Mounted;

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

	// Perform the overlap query
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
		return nullptr; // No overlapping actors
	}

	//print names of overlapping actors

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ACharacterBase* OverlappingCharacter = Cast<ACharacterBase>(OverlapResult.GetActor());
		if (OverlappingCharacter && !TargetEnemies.Contains(OverlappingCharacter))
		{
			if(OverlappingCharacter->GetGenericTeamId() != TeamID)
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

void APlayerCharacter::RotateTowardsLocation(const FVector& TargetLocation)
{
	FVector CurrentLocation = GetActorLocation();

	FVector Direction = TargetLocation - CurrentLocation;
	Direction.Z = 0;
	Direction.Normalize();

	FRotator TargetRotation = Direction.Rotation();

	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 5.0f);

	SetActorRotation(NewRotation);
}


