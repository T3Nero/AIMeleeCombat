// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_BaseCharacter.h"

// Sets default values
AAI_BaseCharacter::AAI_BaseCharacter()
{

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));
	


}

// Called when the game starts or when spawned
void AAI_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}



