// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class AIMELEECOMBAT_API UCharacter_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCharacter_AnimInstance();

protected:

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
	class AAI_BaseCharacter* AICharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
	float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

public:

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;
	
};
