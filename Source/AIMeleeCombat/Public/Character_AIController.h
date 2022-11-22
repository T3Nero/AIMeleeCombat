// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Character_AIController.generated.h"

/**
 * 
 */
UCLASS()
class AIMELEECOMBAT_API ACharacter_AIController : public AAIController
{
	GENERATED_BODY()

public:
	ACharacter_AIController();

	virtual void BeginPlay() override;

	// allows the ai controller to possess the character its attached too
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void PatrolArea();

protected:

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus);

	void AIPerception();

private:

	class UNavigationSystemV1* NavSystem;

	FVector PatrolLocation;

	UPROPERTY(BlueprintReadOnly, Category = "AI Behaviour", meta = (AllowPrivateAccess = "true"))
	class AAI_BaseCharacter* AICharacter;

	class UAISenseConfig_Sight* SightPerception;
	
};
