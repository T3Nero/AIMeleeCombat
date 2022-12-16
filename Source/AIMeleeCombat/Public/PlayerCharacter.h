// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerCombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Attacking UMETA(DisplayName = "Attacking"),
	ECS_Dodging UMETA(DisplayName = "Dodging"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	ECS_Dead UMETA(DisplayName = "Dead"),
	
	ECS_MAX
};

UCLASS()
class AIMELEECOMBAT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Input for moving Forward/Backward
	void MoveForwardBackward(float Value);

	// Input for moving Right/Left
	void MoveRightLeft(float Value);

	// Rotate Controller X-Axis using Mouse * Value for Sensitivity
	void Turn(float Value);

	// Rotate Controller Y-Axis using the Mouse * Value for Sensitivity
	void LookUp(float Value);

	/*
	*  Called via input to turn at a given rate
	*  @param Value	 This is a normalized rate, i.e 1.0 means 100% desired turn rate
	*/
	// Gamepad Input
	void TurnAtRate(float Value);

	/*
	*  Called via input to look up/down at a given rate
	*  @param Value	 This is a normalized rate, i.e 1.0 means 100% desired look at rate
	*/
	// Gamepad Input
	void LookAtRate(float Value);

	// Lerps camera boom in/out when inputing mouse wheel axis forward/backward
	void ZoomCamera(float Value);

	void AttackButtonPressed();

	// Player can perform an attack combo if attack button is continuously pressed 
	void AttackCombo();

	void DodgeButtonPressed();

	void SetMontageToPlay(UAnimMontage* Montage, FName Section) const;

	// Sets CombatState to Unoccupied (Called as an AnimNotify at the end of Montages)
	// Allows player to use next action (attack, dodge etc.)
	UFUNCTION(BlueprintCallable)
	void SetUnoccupied();

	// Compares team numbers to check if target is an enemy
	bool IsEnemy(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void DamageDetectTrace();

	void DamageEnemy(AActor* Enemy);

	void Death();

	void QuitGame();

private:

	// Sets how quickly the player turns left/right
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float MouseTurnSensitivity;

	// Sets how quickly the player looks up/down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float MouseLookSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float GamepadTurnSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float GamepadLookSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float BaseMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	// Used to know which direction the player should dodge in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool IsMovingForward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool IsMovingBackward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool IsMovingRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool IsMovingLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		EPlayerCombatState PlayerCombatState;

	// Index used in AttackCombo() switch to iterate through attack animations (incremented in attack montage combo notify)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		int32 ComboIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool bCanAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool bAttacking;

	// set when currenthp <= 0
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool bIsDead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
		bool bIsDodging;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> AlreadyDamagedActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponents, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponents, meta = (AllowPrivateAccess = "true"))
	USceneComponent* TraceStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponents, meta = (AllowPrivateAccess = "true"))
	USceneComponent* TraceEnd;

	// Team Number determines whether an NPC is friendly/enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		int32 TeamNumber;

	// Current Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		int32 CurrentHP;

	// Max Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		int32 MaxHP;

	// Dodge Animation to play
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
		UAnimMontage* DeathMontage;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE int32 GetTeamNumber() const { return TeamNumber; }
	FORCEINLINE bool GetIsAttacking() const { return bAttacking; }
	FORCEINLINE bool IsDead() const { return bIsDead; }

};
