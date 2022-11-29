// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AIController.h"
#include "AI_BaseCharacter.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"

ACharacter_AIController::ACharacter_AIController()
{
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	AIPerception();

}

void ACharacter_AIController::BeginPlay()
{
	Super::BeginPlay();

}

void ACharacter_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(InPawn == nullptr) { return; }

	AICharacter = Cast<AAI_BaseCharacter>(InPawn);
}

void ACharacter_AIController::PatrolArea()
{
	if(AICharacter)
	{
		if(AICharacter->GetEnemyDetected()) { return; }

		if(NavSystem)
		{
			if(!AICharacter->GetCharacterMovement()->bOrientRotationToMovement)
			{
				AICharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
			}

			NavSystem->K2_GetRandomLocationInNavigableRadius(GetWorld(), AICharacter->GetActorLocation(), PatrolLocation, AICharacter->GetPatrolRadius());
			MoveToLocation(PatrolLocation);
		}
	}
}

void ACharacter_AIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus)
{
	if(AICharacter)
	{
		if(AICharacter->IsEnemy(Actor) && !AICharacter->GetEnemyDetected())
		{
			AAI_BaseCharacter* Enemy = Cast<AAI_BaseCharacter>(Actor);

			AICharacter->SetEnemy(Enemy);
			AICharacter->SetEnemyDetected(true);
		}

		if((Actor->GetActorLocation() - AICharacter->GetActorLocation()).Length() >= SightPerception->LoseSightRadius)
		{
			AICharacter->SetEnemy(nullptr);
			AICharacter->SetEnemyDetected(false);
		}
	}
}

void ACharacter_AIController::AIPerception()
{
	// initialize sight perception
	SightPerception = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Perception"));
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
	SightPerception->SightRadius = 1000.0f;
	SightPerception->LoseSightRadius = SightPerception->SightRadius + 200.0f;
	SightPerception->PeripheralVisionAngleDegrees = 180.0f;
	SightPerception->SetMaxAge(0);
	SightPerception->AutoSuccessRangeFromLastSeenLocation = SightPerception->SightRadius + 500.0f;

	// adds SightPerception Config to PerceptionComponent
	GetPerceptionComponent()->SetDominantSense(*SightPerception->GetSenseImplementation());
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ACharacter_AIController::OnPerceptionUpdated);
	GetPerceptionComponent()->ConfigureSense(*SightPerception);
}


