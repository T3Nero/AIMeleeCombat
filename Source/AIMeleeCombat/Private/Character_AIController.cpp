// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AIController.h"
#include "AI_BaseCharacter.h"
#include "NavigationSystem.h"

ACharacter_AIController::ACharacter_AIController()
{
	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
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


