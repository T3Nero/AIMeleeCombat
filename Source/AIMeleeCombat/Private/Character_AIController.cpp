// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AIController.h"
#include "AI_BaseCharacter.h"
#include "NavigationSystem.h"
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
	PatrolArea();

}

void ACharacter_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(InPawn == nullptr) { return; }

	AICharacter = Cast<AAI_BaseCharacter>(InPawn);
}

void ACharacter_AIController::PatrolArea()
{
	if(AICharacter && AICharacter->CanPatrol())
	{
		if(NavSystem)
		{
			NavSystem->K2_GetRandomLocationInNavigableRadius(GetWorld(), AICharacter->GetActorLocation(), PatrolLocation, AICharacter->GetPatrolRadius());
			MoveToLocation(PatrolLocation);
		}
	}
}

void ACharacter_AIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus)
{
	if(AICharacter->IsEnemy(Actor) && !AICharacter->GetEnemyDetected())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Actor->GetName());
		AICharacter->SeekEnemy(Actor);
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
	SightPerception->SetMaxAge(5.0f);
	SightPerception->AutoSuccessRangeFromLastSeenLocation = SightPerception->SightRadius + 500.0f;

	// adds SightPerception Config to PerceptionComponent
	GetPerceptionComponent()->SetDominantSense(*SightPerception->GetSenseImplementation());
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ACharacter_AIController::OnPerceptionUpdated);
	GetPerceptionComponent()->ConfigureSense(*SightPerception);
}


