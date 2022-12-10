// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_UtilityComponent.h"
#include "AI_BaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"

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


		AICharacter = Cast<AAI_BaseCharacter>(GetOwner());
		
		if(AICharacter)
		{
			AbilitiesAvailable.Add(SeekScore());
			AbilitiesAvailable.Add(StrafeScore());
			AbilitiesAvailable.Add(AttackScore());
			AbilitiesAvailable.Add(RangedAttackScore());
			AbilitiesAvailable.Add(UltimateAttackScore());
			AbilitiesAvailable.Add(DodgeScore());
			AbilitiesAvailable.Add(BlockScore());

			//const float RandWaitTime = UKismetMathLibrary::RandomFloatInRange(1.0f, 2.0f);
			FTimerHandle UpdateScoreTimer;
			GetWorld()->GetTimerManager().SetTimer(UpdateScoreTimer, this, &UAI_UtilityComponent::UpdateScore, 2.0f, true);
		}
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
		// Seek
	case 0:
		if(AICharacter->GetEnemy())
		{
			AICharacter->SeekEnemy(AICharacter->GetEnemy());
		}
		break;

		// Strafe
	case 1:
		AICharacter->StrafeAroundEnemy();
		break;

		// Attack
	case 2:
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		AICharacter->AttackCombo();
		break;

		// Ranged Attack
	case 3:
		UE_LOG(LogTemp, Warning, TEXT("Ranged Attack"));
		AICharacter->RangedAttack();
		break;

		// Ultimate Attack
	case 4:
		UE_LOG(LogTemp, Warning, TEXT("Ultimate Attack"));
		AICharacter->UltimateAttack();
		break;

		// Dodge
	case 5:
		
		break;

		// Block
	case 6:
		
		break;


	default:
		break;

	}


}

void UAI_UtilityComponent::UpdateScore()
{
	if(AICharacter->GetCombatState() != ECombatState::ECS_Unoccupied) { return; }

	//SeekScore();
	//StrafeScore();
	//AttackScore();
	//RangedAttackScore();
	//UltimateAttackScore();
	//BlockScore();
	//DodgeScore();


	for (const float Item : AbilitiesAvailable)
	{
		if(Item == AbilitiesAvailable[0])
		{
			AbilitiesAvailable[0] = SeekScore();
		}
		if(Item == AbilitiesAvailable[1])
		{
			AbilitiesAvailable[1] = StrafeScore();
		}
		if(Item == AbilitiesAvailable[2])
		{
			AbilitiesAvailable[2] = AttackScore();
		}
		if(Item == AbilitiesAvailable[3])
		{
			AbilitiesAvailable[3] = RangedAttackScore();
		}
		if(Item == AbilitiesAvailable[4])
		{
			AbilitiesAvailable[4] = UltimateAttackScore();
		}
		if(Item == AbilitiesAvailable[5])
		{
			AbilitiesAvailable[5] = DodgeScore();
		}
		if(Item == AbilitiesAvailable[6])
		{
			AbilitiesAvailable[6] = BlockScore();
		}
		
	}


	ChooseBestAbility();
}

float UAI_UtilityComponent::ScoreAbilities(TArray<float> Conditions)
{
	float Score = 1;
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

float UAI_UtilityComponent::SeekScore()
{
	TArray<float> SeekConditions;
	float SeekValue = 0;
	if(!AICharacter->InAttackRange())
	{
		SeekValue = 0.9f;
	}
	else
	{
		SeekValue = 0;
	}

	if(AICharacter->InRangedAttackRange())
	{
		SeekValue = 0;
	}

	
	SeekConditions.Add(SeekValue);
	return ScoreAbilities(SeekConditions);
}

float UAI_UtilityComponent::StrafeScore()
{
	TArray<float> StrafeConditions;
	float StrafeValue = 0;
	if(AICharacter->CanStrafe())
	{
		StrafeValue = 0.7;
	}
	else
	{
		StrafeValue = 0;
	}

	StrafeConditions.Add(StrafeValue);

	float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	if(RandNum < StrafeValue)
	{
		return ScoreAbilities(StrafeConditions);
	}
	else
	{
		return 0;
	}
}

float UAI_UtilityComponent::AttackScore()
{
	TArray<float> AttackConditions;
	float AttackRangeValue = 0;

	if(AICharacter->InAttackRange())
	{
		AttackRangeValue = 0.6f;
	}
	else
	{
		AttackRangeValue = 0;
	}

	AttackConditions.Add(AttackRangeValue);

	float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	if(RandNum < S_CombatBehavior->AttackValue)
	{
		return ScoreAbilities(AttackConditions);
	}
	else
	{
		return 0;
	}


	
}

float UAI_UtilityComponent::RangedAttackScore()
{
	TArray<float> RangedAttackConditions;
	float RangedAttackValue = 0;

	
	if(AICharacter->InRangedAttackRange())
	{
		RangedAttackValue = 0.4f;
	}
	else
	{
		RangedAttackValue = 0;
	}

	RangedAttackConditions.Add(RangedAttackValue);

	float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	if(RandNum < S_CombatBehavior->RangedAttackValue)
	{
		return ScoreAbilities(RangedAttackConditions);
	}
	else
	{
		return 0;
	}
}

float UAI_UtilityComponent::UltimateAttackScore()
{
	TArray<float> UltimateAttackConditions;
	float UltimateAttackValue = 0;

	if(AICharacter->InAttackRange())
	{
		UltimateAttackValue = 0.3f;
	}
	else
	{
		UltimateAttackValue = 0;
	}

	UltimateAttackConditions.Add(UltimateAttackValue);


	float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	if(RandNum < S_CombatBehavior->UltimateAttackValue)
	{
		return ScoreAbilities(UltimateAttackConditions);
	}
	else
	{
		return 0;
	}
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


}

