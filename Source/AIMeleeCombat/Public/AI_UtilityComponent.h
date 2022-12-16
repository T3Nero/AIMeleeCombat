// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "AI_UtilityComponent.generated.h"


USTRUCT(BlueprintType)
struct FCombatBehavior : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DodgeValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RangedAttackValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UltimateAttackValue;
	
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AIMELEECOMBAT_API UAI_UtilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAI_UtilityComponent();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	void InitialiseBehavior();

	void ChooseBestAbility();

	UFUNCTION()
	void UpdateScore();

	static float ScoreAbilities(float BehaviorValue, TArray<float> Conditions);

	float SeekScore();

	float StrafeScore();

	float AttackScore();

	float RangedAttackScore();

	float UltimateAttackScore();

	float DodgeScore();

	float BlockScore();


private:

	UPROPERTY()
	class AAI_BaseCharacter* AICharacter;

	FCombatBehavior* S_CombatBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	UDataTable* CombatBehaviorData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	FName RowName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	TArray<float> AbilitiesAvailable;

	FTimerHandle UpdateScoreTimer;
		
};
