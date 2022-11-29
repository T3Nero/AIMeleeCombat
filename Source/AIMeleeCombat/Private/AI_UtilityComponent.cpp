// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_UtilityComponent.h"

// Sets default values for this component's properties
UAI_UtilityComponent::UAI_UtilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


}


// Called when the game starts
void UAI_UtilityComponent::BeginPlay()
{
	Super::BeginPlay();
	InitialiseBehavior();


}

void UAI_UtilityComponent::InitialiseBehavior()
{
	if(CombatBehaviorData)
	{
		const FString ContextString = RowName.ToString();
		S_CombatBehavior = CombatBehaviorData->FindRow<FCombatBehavior>(RowName, ContextString, true);

		AbilitiesAvailable.Add(AttackScore());
		AbilitiesAvailable.Add(RangedAttackScore());
		AbilitiesAvailable.Add(UltimateAttackScore());
		AbilitiesAvailable.Add(DodgeScore());
		AbilitiesAvailable.Add(BlockScore());

	}
}

void UAI_UtilityComponent::ChooseBestAbility()
{
	float Score = 0;
	int BestAbilityIndex = 0;
	for (int i = 0; i < AbilitiesAvailable.Num(); ++i)
	{
		if(AbilitiesAvailable[i] > Score)
		{
			BestAbilityIndex = i;
			Score = AbilitiesAvailable[i];
		}
	}

	switch (BestAbilityIndex)
	{
		// Attack
	case 0:
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		break;

		// Ranged Attack
	case 1:
		UE_LOG(LogTemp, Warning, TEXT("Ranged Attack"));
		break;

		// Ultimate Attack
	case 2:
		break;

		// Dodge
	case 3:
		break;

		// Block
	case 4:
		break;



	default:
		break;

	}


}

float UAI_UtilityComponent::ScoreAbilities(float BehaviorValue, TArray<float> Conditions)
{
	float Score = BehaviorValue;
	for (const float AbilityScore: Conditions)
	{
		Score *= AbilityScore;

		if(Score == 0)
		{
			return 0;
		}
	}

	float OriginalScore = Score;
	float ModFactor = 1 - (1 / Conditions.Num());
	float MakeupValue = (1 - OriginalScore) * ModFactor;
	Score = OriginalScore + (MakeupValue * OriginalScore);
	return Score;
}

float UAI_UtilityComponent::AttackScore()
{
	TArray<float> AttackConditions;
	float InAttackRange = 1.0f;

	AttackConditions.Add(InAttackRange);

	float Score = ScoreAbilities(S_CombatBehavior->AttackValue, AttackConditions);
	return Score;
}

float UAI_UtilityComponent::RangedAttackScore()
{
	TArray<float> RangedAttackConditions;
	float InRangedAttackRange = 0.5f;

	RangedAttackConditions.Add(InRangedAttackRange);

	float Score = ScoreAbilities(S_CombatBehavior->RangedAttackValue, RangedAttackConditions);
	return Score;
}

float UAI_UtilityComponent::UltimateAttackScore()
{
	return 0;
}

float UAI_UtilityComponent::DodgeScore()
{
	return 0;
}

float UAI_UtilityComponent::BlockScore()
{
	return 0;
}

// Called every frame
void UAI_UtilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ChooseBestAbility();
}

