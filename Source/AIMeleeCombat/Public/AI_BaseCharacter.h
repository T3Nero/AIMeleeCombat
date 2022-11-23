// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI_BaseCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Dodging UMETA(DisplayName = "Dodging"),
	ECS_Blocking UMETA(DisplayName = "Blocking"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	
	ECS_MAX
};

UCLASS()
class AIMELEECOMBAT_API AAI_BaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_BaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnAIMoveCompleted(struct FAIRequestID RequestID, const struct FPathFollowingResult &Result);

	UFUNCTION()
	void StopSeekingEnemy();

	UFUNCTION()
	void AttackEnemy();

	void SetupStimulus();

	UFUNCTION()
	void AttackCombo();

	void SetMontageToPlay(UAnimMontage* Montage, FName Section) const;

	// Sets CombatState to Unoccupied so the AI is free to use next action
	UFUNCTION(BlueprintCallable)
	void SetUnoccupied();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionStimuliSourceComponent* Stimulus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	float PatrolRadius;

	// Checks whether the AI are friendly toward each other or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	float TeamNumber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	float WaitTimeTillSeek;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bCanPatrol;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bEnemyDetected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAggressive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 ComboIndex;


	ECombatState CombatState;
	class ACharacter_AIController* Character_AIController;
	AAI_BaseCharacter* EnemyReference;
	FTimerHandle SeekTimerHandle;
	FTimerDelegate SeekDelegate;
	FTimerHandle AttackTimerHandle;


public:

	UFUNCTION()
	void SeekEnemy(AActor* Enemy);

	bool IsEnemy(AActor* Target) const;

	// public getters (allows access to private variables in other classes
	FORCEINLINE float GetPatrolRadius() const { return PatrolRadius; }
	FORCEINLINE bool CanPatrol() const { return bCanPatrol; }
	FORCEINLINE bool GetEnemyDetected() const { return bEnemyDetected; }
};
