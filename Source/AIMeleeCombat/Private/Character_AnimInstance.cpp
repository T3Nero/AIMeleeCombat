// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AnimInstance.h"
#include "AI_BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

// Default Values
UCharacter_AnimInstance::UCharacter_AnimInstance() :
	Speed(0.f),
	Direction(0.f),
	bIsAccelerating(false)
{
}

// BeginPlay()
void UCharacter_AnimInstance::NativeInitializeAnimation()
{
	AICharacter = Cast<AAI_BaseCharacter>(TryGetPawnOwner());
}

// Tick()
void UCharacter_AnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(AICharacter == nullptr)
	{
		AICharacter = Cast<AAI_BaseCharacter>(TryGetPawnOwner());
	}

	if(AICharacter)
	{
		// Set Speed based on Characters current Velocity
		FVector Velocity{ AICharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// Check to see if Character is accelerating (used for transitioning anim state between idle/run)
		if(AICharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		// Sets Characters Direction based on current look rotation & movement rotation
		//FRotator LookRotation = AICharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(AICharacter->GetVelocity());
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AICharacter->GetActorRotation()).Yaw;
		//Direction = UKismetAnimationLibrary::CalculateDirection(AICharacter->GetVelocity(), AICharacter->GetActorRotation());
	}
}