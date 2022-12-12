// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_BaseCharacter.h"
#include "Character_AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character_AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI_UtilityComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
AAI_BaseCharacter::AAI_BaseCharacter() :
	PatrolRadius (1000.f),
	TeamNumber(0),
	CurrentHealth (100.f),
	MaxHealth (100.f),
	bCanPatrol (false),
	StrafeDirection(EStrafeDirection::ESD_NULL),
	AttackRange (250.0f),
	RangedAttackRange(450.0f),
	bEnemyDetected(false),
	bInAttackRange(false),
	bInRangedAttackRange(false),
	bIsAggressive(false),
	bCanStrafe(true),
	bCanBlock(true),
	bCanDodge(true),
	bAttacking(false),
	bIsDead(false),
	ComboIndex(0),
	CombatState(ECombatState::ECS_Unoccupied)

{

	// Creates Weapon Mesh and attaches it to parents skeleton socket
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start"));
	TraceStart->SetupAttachment(Weapon);

	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Trace End"));
	TraceEnd->SetupAttachment(Weapon);

	// Adds utility actor component to AI_BaseCharacter
	UtilityComponent = CreateDefaultSubobject<UAI_UtilityComponent>(TEXT("Utility Component"));

	// Changes Visibility collision channel to block on characters mesh (used for detecting raycast hits when taking damage)
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Required so AIPerception on the AIController detects this character
	SetupStimulus();


}

// Called when the game starts or when spawned
void AAI_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	Character_AIController = Cast<ACharacter_AIController>(GetController());

	// Continues to call OnAIMoveCompleted() once current patrolling has finished & if bCanPatrol = true
	if(Character_AIController)
	{
		Character_AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject
		(this, &AAI_BaseCharacter::OnAIMoveCompleted);
	}
}

void AAI_BaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(EnemyReference && CombatState == ECombatState::ECS_Unoccupied)
	{
		RotateTowardsTarget(EnemyReference->GetActorLocation());
	}

	if(EnemyReference)
	{
		const float EnemyDistance = (EnemyReference->GetActorLocation() - GetActorLocation()).Length();
		if(EnemyDistance > 100 && EnemyDistance <= AttackRange)
		{
			bInAttackRange = true;
		}
		else
		{
			bInAttackRange = false;
		}

		if(EnemyDistance > AttackRange && EnemyDistance <= RangedAttackRange)
		{
			bInRangedAttackRange = true;
		}
		else
		{
			bInRangedAttackRange = false;
		}
	}
}

void AAI_BaseCharacter::OnAIMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	SetUnoccupied();
}

// returns true if targets team number is not equal to owners team number
bool AAI_BaseCharacter::IsEnemy(AActor* Target) const
{
	const AAI_BaseCharacter* Enemy = Cast<AAI_BaseCharacter>(Target);
	if(Enemy && Enemy->TeamNumber != TeamNumber)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float AAI_BaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(CurrentHealth - DamageAmount <= 0.f)
	{
		// Dead
		CurrentHealth = 0;
		Death();
	}
	else
	{
		CurrentHealth -= DamageAmount;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// Character moves towards target enemy until it is within attacking range
void AAI_BaseCharacter::SeekEnemy(AActor* Enemy)
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	if(Character_AIController)
	{
		if(EnemyReference)
		{
			if(bIsAggressive)
			{
				Character_AIController->MoveToLocation(EnemyReference->GetActorLocation(), 200, true);
				SetUnoccupied();
			}
		}
	}
}

void AAI_BaseCharacter::SetupStimulus()
{
	// adds AIPerception component to character and registers sight perception so it can access the AIControllers AIPerception
	Stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus Component"));
	Stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	Stimulus->RegisterWithPerceptionSystem();
}

// Called from AI_UtilityComponent class (ComboIndex given a random integer so the switch chooses a random Attack for the AI to perform)
void AAI_BaseCharacter::AttackCombo()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	if(bInAttackRange)
	{
		CombatState = ECombatState::ECS_Attacking;
		bAttacking = true;
		UAnimMontage* Attack = AttackMontage;
		ComboIndex = UKismetMathLibrary::RandomIntegerInRange(0, 3);

		if(Attack)
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
			SetMontageToPlay(Attack, SectionName);
		}
	}
}

// Called from AI_UtilityComponent class on ChooseBestAbility()
void AAI_BaseCharacter::RangedAttack()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	CombatState = ECombatState::ECS_Attacking;
	bAttacking = true;
	SetMontageToPlay(RangedAttackMontage, "Default");
}

// Called from AI_UtilityComponent class on ChooseBestAbility()
void AAI_BaseCharacter::UltimateAttack()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	CombatState = ECombatState::ECS_Attacking;
	bAttacking = true;
	SetMontageToPlay(UltimateAttackMontage, "Default");
}

void AAI_BaseCharacter::Blocking()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	CombatState = ECombatState::ECS_Blocking;
	SetMontageToPlay(BlockingMontage, "Default");

	bCanBlock = false;
	GetWorld()->GetTimerManager().SetTimer(BlockCooldownHandle, this, &AAI_BaseCharacter::BlockOffCooldown, UKismetMathLibrary::RandomFloatInRange(4, 6), false);
}

void AAI_BaseCharacter::Dodging()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	int32 DodgeIndex = UKismetMathLibrary::RandomIntegerInRange(0, 1);
	if(DodgingMontage)
	{
		FName SectionName;
		switch (ComboIndex)
		{
		case 0:
			SectionName = "DodgeLeft";
			break;
		case 1:
			SectionName = "DodgeRight";
			break;
		default:
			break;
		}

		CombatState = ECombatState::ECS_Dodging;
		SetMontageToPlay(DodgingMontage, SectionName);
	}

	bCanDodge = false;
	GetWorld()->GetTimerManager().SetTimer(BlockCooldownHandle, this, &AAI_BaseCharacter::DodgeOffCooldown, UKismetMathLibrary::RandomFloatInRange(4, 6), false);
	
}

// Called any time we want to perform an animation montage (takes in the montage to perform & montage section as parameters, to know which montage to play) 
void AAI_BaseCharacter::SetMontageToPlay(UAnimMontage* Montage, FName Section)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_JumpToSection(Section);
}

// Called after an action is finished (also called in Anim Notify at the end of Montages)
void AAI_BaseCharacter::SetUnoccupied()
{
	// Reset Attack & CombatState
	ComboIndex = 0;
	bAttacking = false;
	CombatState = ECombatState::ECS_Unoccupied;
	StrafeDirection = EStrafeDirection::ESD_NULL;

	if(!GetCharacterMovement()->bOrientRotationToMovement)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}


void AAI_BaseCharacter::RotateTowardsTarget(FVector Target)
{
	const FVector WorldLocation = GetCapsuleComponent()->GetComponentLocation();
	const FRotator WorldRotation = GetCapsuleComponent()->GetComponentRotation();

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WorldLocation, Target);

	double LerpValue = UKismetMathLibrary::Lerp(WorldRotation.Yaw, LookAtRotation.Yaw, 0.2f);
	const FRotator Rotation = UKismetMathLibrary::MakeRotator(WorldRotation.Roll, WorldRotation.Pitch, LerpValue);
	GetCapsuleComponent()->SetWorldRotation(Rotation);
}

void AAI_BaseCharacter::StrafeOffCooldown()
{
	bCanStrafe = true;
	GetWorld()->GetTimerManager().ClearTimer(StrafeCooldownHandle);
}

void AAI_BaseCharacter::BlockOffCooldown()
{
	bCanBlock = true;
	GetWorld()->GetTimerManager().ClearTimer(BlockCooldownHandle);
}

void AAI_BaseCharacter::DodgeOffCooldown()
{
	bCanDodge = true;
	GetWorld()->GetTimerManager().ClearTimer(DodgeCooldownHandle);
}

void AAI_BaseCharacter::DamageDetectTrace()
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
		if(EnemyReference)
		{
			AActor* ActorHit;
			ActorHit = Hit.GetActor();
			if(AlreadyDamagedActors.Contains(ActorHit) == false)
			{
				AlreadyDamagedActors.AddUnique(ActorHit);
				DamageEnemy(ActorHit);
			}
		}
	}
}

void AAI_BaseCharacter::DamageEnemy(AActor* Enemy)
{
	constexpr float Damage = 20.0f;
	UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
}

void AAI_BaseCharacter::Death()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(1);

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

		CombatState = ECombatState::ECS_Dead;
		bIsDead = true;
		SetMontageToPlay(DeathMontage, SectionName);
	}

	EnemyReference->EnemyReference = nullptr;
	EnemyReference->bEnemyDetected = false;
}

void AAI_BaseCharacter::StrafeAroundEnemy()
{
	if(CombatState != ECombatState::ECS_Unoccupied) { return; }

	if(GetCharacterMovement()->bOrientRotationToMovement)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	if(StrafeDirection == EStrafeDirection::ESD_NULL)
	{
		const float Value = UKismetMathLibrary::RandomFloatInRange(0,1);
		if(Value <= 0.3f)
		{
			StrafeDirection = EStrafeDirection::ESD_Left;
		}
		if(Value > 0.3f && Value <= 0.6f)
		{
			StrafeDirection = EStrafeDirection::ESD_Right;
		}
		else
		{
			StrafeDirection = EStrafeDirection::ESD_Back;
		}
	}

	FVector StrafeDirectionVector;

	switch (StrafeDirection)
	{
	case EStrafeDirection::ESD_Back:
		StrafeDirectionVector = GetActorForwardVector() * -1;
		break;
	case EStrafeDirection::ESD_Left:
		StrafeDirectionVector = GetActorRightVector() * -1;
		break;
	case EStrafeDirection::ESD_Right:
		StrafeDirectionVector = GetActorRightVector();
		break;
	default:
		break;
	}
	const FVector CurrentLocation = GetCapsuleComponent()->GetComponentLocation();
	const FVector Direction = UKismetMathLibrary::RotateAngleAxis((StrafeDirectionVector * 200), 360, FVector(0,0,1));
	const FVector StrafeDestination = (CurrentLocation + Direction);
	Character_AIController->MoveToLocation(StrafeDestination, 0, true);

	GetWorld()->GetTimerManager().SetTimer(StrafeCooldownHandle, this, &AAI_BaseCharacter::StrafeOffCooldown, UKismetMathLibrary::RandomFloatInRange(8, 12), false);
	bCanStrafe = false;

	SetUnoccupied();
}



