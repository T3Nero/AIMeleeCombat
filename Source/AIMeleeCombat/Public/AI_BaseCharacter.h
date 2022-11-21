// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI_BaseCharacter.generated.h"

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

	bool IsEnemy(AActor* Target) const;

	UFUNCTION()
	void SeekEnemy();

	UFUNCTION()
	void StopSeekingEnemy();

	UFUNCTION()
	void OnEnemyDetectedOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnemyDetectedOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* EnemyDetectionSphere;

	class ACharacter_AIController* Character_AIController;

	AAI_BaseCharacter* EnemyReference;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float WaitTimeTillSeek;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool EnemyDetected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;



	FTimerHandle SeekTimerHandle;



public:	

	FORCEINLINE float GetPatrolRadius() const { return PatrolRadius; }
	FORCEINLINE float GetTeamNumber() const { return TeamNumber; }
	FORCEINLINE bool CanPatrol() const { return bCanPatrol; }
};
