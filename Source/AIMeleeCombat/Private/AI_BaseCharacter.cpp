// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_BaseCharacter.h"
#include "Character_AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

// Sets default values
AAI_BaseCharacter::AAI_BaseCharacter() :
	PatrolRadius (1000.f),
	TeamNumber(0),
	CurrentHealth (100.f),
	MaxHealth (100.f),
	WaitTimeTillSeek (0.5f),
	bCanPatrol (false),
	AttackRange (150.0f),
	EnemyDetected(false),
	bInAttackRange(false),
	bShouldAttack(false),
	bIsAggressive(false),
	ComboIndex(0),
	CombatState(ECombatState::ECS_Unoccupied)

{

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));

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

}

// Calls PatrolArea() from AIController class
void AAI_BaseCharacter::OnAIMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if(!EnemyDetected)
	{
		Character_AIController->PatrolArea();
	}
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
		EnemyDetected = true;
		EnemyReference = Cast<AAI_BaseCharacter>(Enemy);

		if(EnemyReference)
		{
			if(CombatState == ECombatState::ECS_Unoccupied)
			{
				AttackEnemy();
			}

			if(bIsAggressive)
			{
				Character_AIController->MoveToLocation(EnemyReference->GetActorLocation(), AttackRange, true);
			}

			// Continues to call SeekEnemy() every "WaitTimeTillSeek" seconds
			const FTimerDelegate SeekDelegate = FTimerDelegate::CreateUObject(this, &AAI_BaseCharacter::SeekEnemy, Enemy);
			GetWorld()->GetTimerManager().SetTimer(SeekTimerHandle, SeekDelegate, WaitTimeTillSeek, true);
		}
	}
}

void AAI_BaseCharacter::StopSeekingEnemy()
{
	// Clears the timer handle so SeekEnemy() stops being called
	GetWorld()->GetTimerManager().ClearTimer(SeekTimerHandle);
}

void AAI_BaseCharacter::AttackEnemy()
{
	// in attack range, stop seeking, attack
	if((EnemyReference->GetActorLocation() - GetActorLocation()).Length() <= (AttackRange + 50))
	{
		StopSeekingEnemy();
		bInAttackRange = true;
		bShouldAttack = true;
		UE_LOG(LogTemp, Warning, TEXT("In Attack Range"));

		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAI_BaseCharacter::AttackCombo, 0.5f, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
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
	bShouldAttack = true;
	CombatState = ECombatState::ECS_Unoccupied;
}



