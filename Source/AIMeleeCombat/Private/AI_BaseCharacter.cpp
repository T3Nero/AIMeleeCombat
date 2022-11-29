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

// Sets default values
AAI_BaseCharacter::AAI_BaseCharacter() :
	PatrolRadius (1000.f),
	TeamNumber(0),
	CurrentHealth (100.f),
	MaxHealth (100.f),
	bCanPatrol (false),
	AttackRange (150.0f),
	bEnemyDetected(false),
	bInAttackRange(false),
	bShouldAttack(false),
	bIsAggressive(false),
	ComboIndex(0),
	StrafeDirection(EStrafeDirection::ESD_NULL),
	CombatState(ECombatState::ECS_Unoccupied),
	PatrolValue(0.2f),
	SeekValue(0.5f),
	StrafeValue(0.8f)

{

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	UtilityComponent = CreateDefaultSubobject<UAI_UtilityComponent>(TEXT("Utility Component"));

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
	if(Character_AIController && bCanPatrol)
	{
		Character_AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject
		(this, &AAI_BaseCharacter::OnAIMoveCompleted);
	}

	WeightValues.AddUnique(PatrolValue);
	WeightValues.AddUnique(SeekValue);
	WeightValues.AddUnique(StrafeValue);

	AttackEnemy();
}

void AAI_BaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(bEnemyDetected)
	{
		RotateTowardsTarget(EnemyReference->GetActorLocation());
	}
	ChooseSteeringBehavior();
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

// Character moves towards target enemy until it is in attack range
void AAI_BaseCharacter::SeekEnemy(AActor* Enemy)
{
	if(Character_AIController)
	{
		if(EnemyReference)
		{
			if(bIsAggressive)
			{
				Character_AIController->MoveToLocation(EnemyReference->GetActorLocation(), AttackRange, true);
			}
		}
	}
}

void AAI_BaseCharacter::AttackEnemy()
{
	float RandWaitTime = UKismetMathLibrary::RandomFloatInRange(1.5f, 3.0f);
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAI_BaseCharacter::AttackEnemy, RandWaitTime, true);
	if(EnemyReference == nullptr) { return; }

	// in attack range, stop seeking, attack
	if((EnemyReference->GetActorLocation() - GetActorLocation()).Length() <= (AttackRange + 50))
	{
		bShouldAttack = true;
		bInAttackRange = true;
		AttackCombo();
	}
	else
	{
		bShouldAttack = false;
		bInAttackRange = false;
		SetUnoccupied();
	}
}

void AAI_BaseCharacter::SetupStimulus()
{
	Stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus Component"));
	Stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	Stimulus->RegisterWithPerceptionSystem();
}

void AAI_BaseCharacter::AttackCombo()
{
	if(bInAttackRange && bShouldAttack)
	{
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
			bShouldAttack = false;
			CombatState = ECombatState::ECS_Attacking;
			SetMontageToPlay(Attack, SectionName);
		}
	}
}

void AAI_BaseCharacter::SetMontageToPlay(UAnimMontage* Montage, FName Section) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(Montage);
	AnimInstance->Montage_JumpToSection(Section);
}

void AAI_BaseCharacter::SetUnoccupied()
{
	// Reset Attack & CombatState
	ComboIndex = 0;
	CombatState = ECombatState::ECS_Unoccupied;
	StrafeDirection = EStrafeDirection::ESD_NULL;
}

void AAI_BaseCharacter::RotateTowardsTarget(FVector Target)
{
	if(EnemyReference == nullptr) { return; }

	const FVector WorldLocation = GetCapsuleComponent()->GetComponentLocation();
	const FRotator WorldRotation = GetCapsuleComponent()->GetComponentRotation();

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WorldLocation, Target);

	double LerpValue = UKismetMathLibrary::Lerp(WorldRotation.Yaw, LookAtRotation.Yaw, 0.5f);
	const FRotator Rotation = UKismetMathLibrary::MakeRotator(WorldRotation.Roll, WorldRotation.Pitch, LerpValue);
	GetCapsuleComponent()->SetWorldRotation(Rotation);
}

void AAI_BaseCharacter::StrafeAroundEnemy()
{
	if((EnemyReference->GetActorLocation() - GetActorLocation()).Length() <= (AttackRange + 200))
	{
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
		FVector Direction = UKismetMathLibrary::RotateAngleAxis((StrafeDirectionVector * 200), 360, FVector(0,0,1));
		FVector StrafeDestination = (CurrentLocation + Direction);

		Character_AIController->MoveToLocation(StrafeDestination, 10, true);
		SetUnoccupied();
	}
}

void AAI_BaseCharacter::ChooseSteeringBehavior()
{
	if(CombatState != ECombatState::ECS_Unoccupied) {return;}

	int32 ChosenIndex = 1;
	float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	for (float Value : WeightValues)
	{
		if(RandNum < Value)
		{
			ChosenIndex = WeightValues.IndexOfByKey(Value);
			break;
		}
	}

	switch(ChosenIndex)
	{
		case 0:
			if(!bCanPatrol) { return; }

			Character_AIController->PatrolArea();
			CombatState = ECombatState::ECS_Patrol;
		break;
		case 1:
			if(EnemyReference == nullptr || bInAttackRange)
			{
				return;
			}
			SeekEnemy(EnemyReference);
			CombatState = ECombatState::ECS_Seek;
		break;
		case 2:
			if(EnemyReference == nullptr) { return; }

			StrafeAroundEnemy();
			CombatState = ECombatState::ECS_Strafe;
		break;
		default:
		break;
	}
}



