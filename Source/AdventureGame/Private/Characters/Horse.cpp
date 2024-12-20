// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Horse.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Characters/PlayerCharacter.h"

// Sets default values
AHorse::AHorse()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	TPSCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	TPSCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TPSCameraComponent->bUsePawnControlRotation = false;

	CurrentState = EHorseState::Idle;

	WalkSpeed = 150.0f;
	GallopSpeed = 600.0f;

	MountColliderLeft = CreateDefaultSubobject<USphereComponent>(TEXT("MountColliderLeft"));
	MountColliderRight = CreateDefaultSubobject<USphereComponent>(TEXT("MountColliderRight"));
	MountColliderLeft->SetupAttachment(RootComponent);
	MountColliderRight->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHorse::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = GallopSpeed;
}

void AHorse::PossessedBy(AController* NewController)
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

// Called every frame
void AHorse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentYaw = GetActorRotation().Yaw;

	float DeltaYaw = CurrentYaw - PreviousYaw;

	PreviousYaw = CurrentYaw;

	if (DeltaYaw > 180.0f)
	{
		DeltaYaw -= 360.0f;
	}
	else if (DeltaYaw < -180.0f)
	{
		DeltaYaw += 360.0f;
	}

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Delta Yaw: %f"), DeltaYaw));

	//print velocity size
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Velocity Size: %f"), GetVelocity().Size()));

	if (FMath::Abs(DeltaYaw) > 1.f)
	{
		if(DeltaYaw < 0)
		{
			AnimMovementDirection = FMath::FInterpTo(AnimMovementDirection, -100, DeltaTime, 5);
		}
		else
		{
			AnimMovementDirection = FMath::FInterpTo(AnimMovementDirection, 100, DeltaTime, 5);
		}
	}
	else
	{
		AnimMovementDirection = FMath::FInterpTo(AnimMovementDirection, 0, DeltaTime, 5);
	}

	if (FMath::Abs(DeltaYaw) > 2.f)
	{
		GetCharacterMovement()->MaxWalkSpeed = 800;
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Speed: %f"), GetCharacterMovement()->MaxWalkSpeed));

		FVector CurrentVelocity = GetCharacterMovement()->Velocity;

		if (CurrentVelocity.Size() > 800)
		{
			FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * 800;
			GetCharacterMovement()->Velocity = ClampedVelocity;
		}

		AnimPlayRate = 1.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 1200;
		AnimPlayRate = 1.2f;
	}


	if (bMoveInput)
	{
		if (bIsWalking)
		{
			CurrentState = EHorseState::Walk;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		else
		{
			CurrentState = EHorseState::Gallop;
			GetCharacterMovement()->MaxWalkSpeed = GallopSpeed;
		}
	}
	else
	{
		CurrentState = EHorseState::Idle;
	}
}

// Called to bind functionality to input
void AHorse::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHorse::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AHorse::LookEnd);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHorse::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHorse::MoveEnd);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &AHorse::Walk);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &AHorse::WalkEnd);
	}
}

void AHorse::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);

	bLookInput = true;
}

void AHorse::LookEnd(const FInputActionValue& Value)
{
	bLookInput = false;
}

void AHorse::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0, ControlRotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, MovementVector.Y);

	AddMovementInput(RightDirection, MovementVector.X);

	if(!FMath::IsNearlyZero(MovementVector.X) && !bLookInput)
	{
		AddControllerYawInput(MovementVector.X * 0.25);	
	}
	
	bMoveInput = true;
}

void AHorse::MoveEnd(const FInputActionValue& Value)
{
	bMoveInput = false;

	if (BreakMontage && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(BreakMontage);
	}
}

void AHorse::Walk(const FInputActionValue& Value)
{
	if(!bIsWalking)
	{
		bIsWalking = true;
	}
}

void AHorse::WalkEnd(const FInputActionValue& Value)
{
	bIsWalking = false;
}

