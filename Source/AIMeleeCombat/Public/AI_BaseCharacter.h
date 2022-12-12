// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI_BaseCharacter.generated.h"

// Combat States are set so actions cant be performed whilst another action is already being performed (must be Unoccupied before performing next action)
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Dodging UMETA(DisplayName = "Dodging"),
	ECS_Blocking UMETA(DisplayName = "Blocking"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	ECS_Patrol UMETA(DisplayName = "Patrol"),
	ECS_Seek UMETA(DisplayName = "Seek"),
	ECS_Strafe UMETA(DisplayName = "Strafe"),
	ECS_Dead UMETA(DisplayName = "Dead"),
	
	ECS_MAX
};

UENUM(BlueprintType)
enum class EStrafeDirection : uint8
{
	ESD_NULL UMETA(DisplayName = "Null"),
	ESD_Back UMETA(DisplayName = "Back"),
	ESD_Left UMETA(DisplayName = "Left"),
	ESD_Right UMETA(DisplayName = "Right"),

	ESD_MAX
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

	virtual void Tick(float DeltaSeconds) override;

	void OnAIMoveCompleted(struct FAIRequestID RequestID, const struct FPathFollowingResult &Result);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAI_UtilityComponent* UtilityComponent;

	void SetupStimulus();

	void SetMontageToPlay(UAnimMontage* Montage, FName Section);

	// Sets CombatState to Unoccupied so the AI is free to use next action
	UFUNCTION(BlueprintCallable)
	void SetUnoccupied();

	void RotateTowardsTarget(FVector Target);

	UFUNCTION()
	void StrafeOffCooldown();

	UFUNCTION()
	void BlockOffCooldown();

	UFUNCTION()
	void DodgeOffCooldown();

	UFUNCTION(BlueprintCallable)
	void DamageDetectTrace();

	void DamageEnemy(AActor* Enemy);

	// Called in "TakeDamage(...)" once CurrentHealth = 0 // Plays Death Montage, clears current target enemy & sets combat state to Dead
	void Death();

	

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
	bool bCanPatrol;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	EStrafeDirection StrafeDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RangedAttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bEnemyDetected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInRangedAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAggressive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bCanStrafe;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bCanBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bCanDodge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponents, meta = (AllowPrivateAccess = "true"))
	USceneComponent* TraceStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponents, meta = (AllowPrivateAccess = "true"))
	USceneComponent* TraceEnd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* RangedAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* UltimateAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* BlockingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DodgingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	int32 ComboIndex;

	// Checks if actor has been damaged so we only damage once during the current attack (cleared in anim notify after attack)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Runtime, meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> AlreadyDamagedActors;


	ECombatState CombatState;
	class ACharacter_AIController* Character_AIController;
	AAI_BaseCharacter* EnemyReference;
	FTimerHandle AttackTimerHandle;
	FTimerHandle StrafeCooldownHandle;
	FTimerHandle BlockCooldownHandle;
	FTimerHandle DodgeCooldownHandle;


public:

	void SeekEnemy(AActor* Enemy);

	void StrafeAroundEnemy();

	void AttackCombo();

	void RangedAttack();

	void UltimateAttack();

	void Blocking();

	void Dodging();

	bool IsEnemy(AActor* Target) const;

	// overriden from actor class
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// public getters (allows access to private variables in other classes)
	FORCEINLINE float GetPatrolRadius() const { return PatrolRadius; }
	FORCEINLINE bool CanPatrol() const { return bCanPatrol; }
	FORCEINLINE bool GetEnemyDetected() const { return bEnemyDetected; }
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE AAI_BaseCharacter* GetEnemy() const { return EnemyReference; }
	FORCEINLINE bool InAttackRange() const { return bInAttackRange; }
	FORCEINLINE bool InRangedAttackRange() const { return bInRangedAttackRange; }
	FORCEINLINE bool CanStrafe() const { return bCanStrafe; }
	FORCEINLINE bool CanBlock() const { return bCanBlock; }
	FORCEINLINE bool CanDodge() const { return bCanDodge; }
	FORCEINLINE bool GetIsAttacking() const { return bAttacking; }
	FORCEINLINE bool IsEnemyDead() const { return bIsDead; }

	// public setters (allows access to private variables in other classes)
	FORCEINLINE void SetEnemy(AAI_BaseCharacter* Enemy) {EnemyReference = Enemy;}
	FORCEINLINE void SetEnemyDetected(bool ED) {bEnemyDetected = ED;}
};
