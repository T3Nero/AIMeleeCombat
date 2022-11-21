// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_BaseCharacter.h"
#include "Character_AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITypes.h"
#include "Components/SphereComponent.h"

// Sets default values
AAI_BaseCharacter::AAI_BaseCharacter() :
	PatrolRadius (1000.f),
	CurrentHealth (100.f),
	MaxHealth (100.f),
	bCanPatrol (true),
	AttackRange (150.0f),
	WaitTimeTillSeek (0.5f)
{

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));

	EnemyDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Enemy Detection Sphere"));
	EnemyDetectionSphere->SetupAttachment(RootComponent);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	EnemyDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAI_BaseCharacter::OnEnemyDetectedOverlapBegin);
	EnemyDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AAI_BaseCharacter::OnEnemyDetectedOverlapEnd);


}

// Called when the game starts or when spawned
void AAI_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	Character_AIController = Cast<ACharacter_AIController>(GetController());

	if(Character_AIController && bCanPatrol)
	{
		Character_AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject
		(this, &AAI_BaseCharacter::OnAIMoveCompleted);
	}

}


void AAI_BaseCharacter::OnAIMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if(!EnemyDetected)
	{
		Character_AIController->PatrolArea();
	}
	else if(EnemyDetected && bInAttackRange)
	{
		StopSeekingEnemy();
	}
}

// returns true if OtherActors team number is not equal to owners team number
bool AAI_BaseCharacter::IsEnemy(AActor* Target) const
{
	const AAI_BaseCharacter* Enemy = Cast<AAI_BaseCharacter>(Target);
	if(Enemy && Enemy->GetTeamNumber() != TeamNumber)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AAI_BaseCharacter::SeekEnemy()
{
	if(Character_AIController)
	{
		Character_AIController->MoveToLocation(EnemyReference->GetActorLocation(), AttackRange, true);

		// Continues to call SeekEnemy() every "WaitTimeTillSeek" seconds
		GetWorld()->GetTimerManager().SetTimer(SeekTimerHandle, this, &AAI_BaseCharacter::SeekEnemy, WaitTimeTillSeek, true);

	}
}

void AAI_BaseCharacter::StopSeekingEnemy()
{
	// Clears the timer handle so SeekEnemy() stops being called
	GetWorld()->GetTimerManager().ClearTimer(SeekTimerHandle);
}

void AAI_BaseCharacter::OnEnemyDetectedOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(IsEnemy(OtherActor))
	{
		EnemyReference = Cast<AAI_BaseCharacter>(OtherActor);
		if(EnemyReference)
		{
			EnemyDetected = true;
			SeekEnemy();
		}
	}
}

void AAI_BaseCharacter::OnEnemyDetectedOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(IsEnemy(OtherActor))
	{
		EnemyReference = Cast<AAI_BaseCharacter>(OtherActor);
		if(EnemyReference)
		{
			EnemyDetected = false;
			StopSeekingEnemy();
			Character_AIController->PatrolArea();
		}
	}
}



