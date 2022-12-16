// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_UtilityComponent.h"
#include "AI_BaseCharacter.h"
#include "PlayerCharacter.h"
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

			
			GetWorld()->GetTimerManager().SetTimer(UpdateScoreTimer, this, &UAI_UtilityComponent::UpdateScore, 0.5f, true);
		}
	}
}

void UAI_UtilityComponent::ChooseBestAbility()
{
	//float Score = 0;
	int BestAbilityIndex = 0;
	const float RandNum = UKismetMathLibrary::RandomFloatInRange(0, 1);
	for (int i = 0; i < AbilitiesAvailable.Num(); ++i)
	{
		// Checks that the ability has a score value > 0
		if(AbilitiesAvailable[i] > 0)
		{
			for (const float Item : AbilitiesAvailable)
			{
				// Prioritized Dithering
				if(RandNum < Item)
				{
					// Checks if the [i] ability score is higher than the current score & calls the switch case relating to that index (original utility code)
					//if(AbilitiesAvailable[i] > Score)
					//{
					//	BestAbilityIndex = i;
					//	Score = AbilitiesAvailable[i];
					//}

					// Sets index to = the element relating to Item in the AbilitiesAvailable array
					BestAbilityIndex = AbilitiesAvailable.IndexOfByKey(Item);
				}
			}
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
		if(AICharacter->GetEnemyPlayer())
		{
			AICharacter->SeekEnemy(AICharacter->GetEnemyPlayer());
		}
		break;

		// Strafe
	case 1:
		AICharacter->StrafeAroundEnemy();
		break;

		// Attack
	case 2:
		AICharacter->AttackCombo();
		break;

		// Ranged Attack
	case 3:
		AICharacter->RangedAttack();
		break;

		// Ultimate Attack
	case 4:
		AICharacter->UltimateAttack();
		break;

		// Dodge
	case 5:
		AICharacter->Dodging();
		break;

		// Block
	case 6:
		AICharacter->Blocking();
		break;


	default:
		break;

	}


}

void UAI_UtilityComponent::UpdateScore()
{
	if(AICharacter->IsDead())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateScoreTimer);
		return;
	}

	if(AICharacter->GetCombatState() != ECombatState::ECS_Unoccupied) { return; }

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

	// Averaging scheme of overall score
	// Depending on the amount of conditions being multiplied, Score can become very small,
	// so this calculation averages it out giving it a more readable score
	const float OriginalScore = Score;
	const float ModFactor = 1 - (1 / Conditions.Num());
	const float MakeupValue = (1 - OriginalScore) * ModFactor;
	Score = OriginalScore + (MakeupValue * OriginalScore);
	return Score;
}

float UAI_UtilityComponent::SeekScore()
{
	TArray<float> SeekConditions;
	float SeekValue = 0;
	if(!AICharacter->InAttackRange() && AICharacter->GetEnemyDetected())
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
	return ScoreAbilities(SeekValue, SeekConditions);
}

float UAI_UtilityComponent::StrafeScore()
{
	TArray<float> StrafeConditions;
	float StrafeValue = 0;
	if(AICharacter->CanStrafe() && AICharacter->GetEnemyDetected())
	{
		StrafeValue = 0.8;
	}
	else
	{
		StrafeValue = 0;
	}

	StrafeConditions.Add(StrafeValue);


	return ScoreAbilities(StrafeValue, StrafeConditions);
}

float UAI_UtilityComponent::AttackScore()
{
	TArray<float> AttackConditions;
	float AttackRangeValue = 0;

	if(AICharacter->InAttackRange() && AICharacter->GetEnemyDetected())
	{
		AttackRangeValue = 0.6f;
	}
	else
	{
		AttackRangeValue = 0;
	}



	AttackConditions.Add(AttackRangeValue);


	return ScoreAbilities(S_CombatBehavior->AttackValue, AttackConditions);
}

float UAI_UtilityComponent::RangedAttackScore()
{
	TArray<float> RangedAttackConditions;
	float RangedAttackValue = 0;

	
	if(AICharacter->InRangedAttackRange() && AICharacter->GetEnemyDetected())
	{
		RangedAttackValue = 0.3f;
	}
	else
	{
		RangedAttackValue = 0;
	}

	RangedAttackConditions.Add(RangedAttackValue);


	return ScoreAbilities(S_CombatBehavior->RangedAttackValue, RangedAttackConditions);
}

float UAI_UtilityComponent::UltimateAttackScore()
{
	TArray<float> UltimateAttackConditions;
	float UltimateAttackValue = 0;

	if(AICharacter->InAttackRange() && AICharacter->GetEnemyDetected())
	{
		UltimateAttackValue = 0.2f;
	}
	else
	{
		UltimateAttackValue = 0;
	}

	UltimateAttackConditions.Add(UltimateAttackValue);



	return ScoreAbilities(S_CombatBehavior->UltimateAttackValue, UltimateAttackConditions);
}

// Character attempts to Dodge incoming ranged melee attacks
float UAI_UtilityComponent::DodgeScore()
{
	TArray<float> DodgeConditions;
	float DodgeValue = 0;

	if(AICharacter->GetEnemy())
	{
		if(AICharacter->GetEnemy()->GetIsAttacking() && AICharacter->CanDodge())
		{
			DodgeValue = 0.5f;
		}
		else
		{
			DodgeValue = 0;
		}
	}

	if(AICharacter->GetEnemyPlayer())
	{
		if(AICharacter->GetEnemyPlayer()->GetIsAttacking() && AICharacter->CanDodge())
		{
			DodgeValue = 0.5f;
		}
		else
		{
			DodgeValue = 0;
		}
	}




	DodgeConditions.Add(DodgeValue);
	return ScoreAbilities(S_CombatBehavior->DodgeValue, DodgeConditions);
}


// Character attempts to Block incoming melee attacks
float UAI_UtilityComponent::BlockScore()
{
	TArray<float> BlockConditions;
	float BlockValue = 0;

	if(AICharacter->GetEnemy())
	{
		if(AICharacter->GetEnemy()->GetIsAttacking() && AICharacter->CanBlock())
		{
			BlockValue = 0.6f;
		}
		else
		{
			BlockValue = 0;
		}
	}

	if(AICharacter->GetEnemyPlayer())
	{
		if(AICharacter->GetEnemyPlayer()->GetIsAttacking() && AICharacter->CanBlock())
		{
			BlockValue = 0.6f;
		}
		else
		{
			BlockValue = 0;
		}
	}

	BlockConditions.Add(BlockValue);
	return ScoreAbilities(S_CombatBehavior->BlockValue, BlockConditions);
}

