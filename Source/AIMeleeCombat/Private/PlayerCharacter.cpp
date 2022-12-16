// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework//SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AI_BaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter() :
	MouseTurnSensitivity(20.f),
	MouseLookSensitivity(20.f),
	GamepadTurnSensitivity(70.f),
	GamepadLookSensitivity(70.f),
	BaseMovementSpeed(400.f),
	PlayerCombatState(EPlayerCombatState::ECS_Unoccupied),
	ComboIndex(0),
	bCanAttack(true),
	bAttacking(false),
	bIsDead(false),
	bIsDodging(false),
	TeamNumber(0),
	CurrentHP(200),
	MaxHP(200)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates a Camera Boom / Spring Arm (Sets Distance from Player to Camera)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // Distance away from Player
	CameraBoom->bUsePawnControlRotation = true; // Rotate spring arm based on player movement
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 100.f); // Offsets position of camera behind player

	// Creates Follow Camera behind Player
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Sets the follow camera to be a child of Camera Boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to spring arm

	// Set Yaw Rotation to True for strafing left/right
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start"));
	TraceStart->SetupAttachment(Weapon);

	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Trace End"));
	TraceEnd->SetupAttachment(Weapon);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	
}

void APlayerCharacter::MoveForwardBackward(float Value)
{
	if(PlayerCombatState == EPlayerCombatState::ECS_Unoccupied || PlayerCombatState == EPlayerCombatState::ECS_Dodging)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
				// Calculate which X Axis Direction to move in based on Controllers Yaw Rotation
				const FRotator Rotation{ Controller->GetControlRotation() };
				const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

				const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
				AddMovementInput(Direction, Value);

				if (Value > 0)
				{
					IsMovingForward = true;
				}
				else if (Value < 0)
				{
					IsMovingBackward = true;
				}
		}
		else if (Value == 0)
		{
			IsMovingForward = false;
			IsMovingBackward = false;
		}
	}
}

void APlayerCharacter::MoveRightLeft(float Value)
{
	if(PlayerCombatState == EPlayerCombatState::ECS_Unoccupied || PlayerCombatState == EPlayerCombatState::ECS_Dodging)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
			{
				// Calculate which Y Axis Direction to move in based on Controllers Yaw Rotation
				const FRotator Rotation{ Controller->GetControlRotation() };
				const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

				const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
				AddMovementInput(Direction, Value);

				if (Value > 0)
				{
					IsMovingRight = true;
				}
				else if (Value < 0)
				{
					IsMovingLeft = true;
				}
		}
		else if (Value == 0)
		{
			IsMovingLeft = false;
			IsMovingRight = false;
		}
	}
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * MouseTurnSensitivity * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * MouseLookSensitivity * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * GamepadTurnSensitivity * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookAtRate(float Value)
{
	AddControllerPitchInput(Value * GamepadLookSensitivity * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::ZoomCamera(float Value)
{
	if (CameraBoom && Value != 0)
	{
		const float CurrentArmLength = CameraBoom->TargetArmLength;
		float ZoomValue = CurrentArmLength - Value * 100.f;
		ZoomValue = FMath::Clamp(ZoomValue, 250.f, 1000.f);
		CameraBoom->TargetArmLength = FMath::FInterpTo(CurrentArmLength, ZoomValue, UGameplayStatics::GetWorldDeltaSeconds(this), 20.f);
	}
}

void APlayerCharacter::AttackButtonPressed()
{
	if(GetCharacterMovement()->IsFalling()) {return; }
	if(PlayerCombatState == EPlayerCombatState::ECS_Dodging) {return;}

	AttackCombo();
}

void APlayerCharacter::AttackCombo()
{
	if (bCanAttack)
	{
		if (AttackMontage)
		{
			FName SectionName;
			switch (ComboIndex)
			{
			case 0:
				SectionName = "Attack01";
				break;
			case 1:
				SectionName = "Attack02";
				break;
			case 2:
				SectionName = "Attack03";
				break;
			case 3:
				SectionName = "Attack04";
				break;
			default:
				SectionName = "Attack01";
				break;
			}
			PlayerCombatState = EPlayerCombatState::ECS_Attacking;
			bAttacking = true;
			bCanAttack = false;
			SetMontageToPlay(AttackMontage, SectionName);
		}
		
	}
}

void APlayerCharacter::DodgeButtonPressed()
{
	if(PlayerCombatState != EPlayerCombatState::ECS_Unoccupied) {return;}

	// Dodge animations are split into sections in the DodgeMontage (section name determines which dodge animation to play)
	FName SectionName = "Forward";

	if (IsMovingForward)
	{
		SectionName = "Forward";
	}
	if (IsMovingBackward)
	{
		SectionName = "Backward";
	}
	if (IsMovingLeft)
	{
		SectionName = "Left";
	}
	if (IsMovingRight)
	{
		SectionName = "Right";
	}

	// Sets collisions to ignore so the player can pass through enemies when dodging & does not take damage
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);

	SetMontageToPlay(DodgeMontage, SectionName);
	bIsDodging = true;
	PlayerCombatState = EPlayerCombatState::ECS_Dodging;
}

void APlayerCharacter::SetMontageToPlay(UAnimMontage* Montage, FName Section) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_JumpToSection(Section);
}

void APlayerCharacter::SetUnoccupied()
{
	// Resets Attack
	ComboIndex = 0;
	bCanAttack = true;
	bAttacking = false;
	bIsDodging = false;

	// Sets collisions back to block so the player can take damage again after dodging
	if(PlayerCombatState == EPlayerCombatState::ECS_Dodging)
	{
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Block);
	}

	// If player is dead, disable input on the player controller to stop player from using any actions
	if(PlayerCombatState == EPlayerCombatState::ECS_Dead)
	{
		GetMesh()->bPauseAnims = true;
		APlayerController* PC =  UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			DisableInput(PC);
		}
	}
	else
	{
		PlayerCombatState = EPlayerCombatState::ECS_Unoccupied;
	}
}

bool APlayerCharacter::IsEnemy(AActor* Target)
{
	AAI_BaseCharacter* Enemy = Cast<AAI_BaseCharacter>(Target);

	if(Enemy && TeamNumber != Enemy->GetTeamNumber())
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

void APlayerCharacter::DamageDetectTrace()
{
	const ETraceTypeQuery TraceParams = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.AddUnique(this);
	FHitResult Hit;

	const FVector StartLocation = TraceStart->GetComponentLocation();
	const FVector EndLocation = TraceEnd->GetComponentLocation();

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, 20.f, TraceParams, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true, FColor::Blue, FColor::Green, 2.0f);
	if(bHit)
	{
		AActor* ActorHit;
		ActorHit = Hit.GetActor();
		if(IsEnemy(ActorHit))
		{
			if(AlreadyDamagedActors.Contains(ActorHit) == false)
			{
				AlreadyDamagedActors.AddUnique(ActorHit);
				DamageEnemy(ActorHit);
			}
		}
	}
}

void APlayerCharacter::DamageEnemy(AActor* Enemy)
{
	constexpr float Damage = 20.0f;
	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
}

void APlayerCharacter::Death()
{
	if(bIsDead) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.1f);

	const int32 DeathIndex = UKismetMathLibrary::RandomIntegerInRange(0, 1);
	if(DeathMontage)
	{
		FName SectionName;
		switch (DeathIndex)
		{
		case 0:
			SectionName = "Death01";
			break;
		case 1:
			SectionName = "Death02";
			break;
		default:
			break;
		}

		SetMontageToPlay(DeathMontage, SectionName);
	}

	bIsDead = true;
	PlayerCombatState = EPlayerCombatState::ECS_Dead;
}

void APlayerCharacter::QuitGame()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, true);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(CurrentHP - DamageAmount <= 0.f)
	{
		// Dead
		CurrentHP = 0;
		Death();
	}
	else
	{
		// Player avoids taking damage if they are dodging
		if(bIsDodging)
		{
			DamageAmount = 0;
		}

		CurrentHP -= DamageAmount;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Mouse/Gamepad Axis Inputs
	PlayerInputComponent->BindAxis(TEXT("MoveForward/Backward"), this, &APlayerCharacter::MoveForwardBackward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight/Left"), this, &APlayerCharacter::MoveRightLeft);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("TurnAtRate"), this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("LookAtRate"), this, &APlayerCharacter::LookAtRate);
	PlayerInputComponent->BindAxis(TEXT("Zoom"), this, &APlayerCharacter::ZoomCamera);

	// Mouse/Gamepad Action Inputs
	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &APlayerCharacter::AttackButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Dodge"), EInputEvent::IE_Pressed, this, &APlayerCharacter::DodgeButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Quit"), EInputEvent::IE_Pressed, this, &APlayerCharacter::QuitGame);
}

