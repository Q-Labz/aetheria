// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaTitheCruiser.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaTitheCruiser::AAetheriaTitheCruiser()
{
	DisplayName = FText::FromString(TEXT("Picket Cruiser 'Tithe of Sector 7'"));
	Faction = E_Faction::VanceRemnant;
	bTargetsFlagship = true;

	MaxHull = 4000.0f;
	MaxShield = 1500.0f;
	MoveSpeed = 500.0f;
	WeaponDamage = 90.0f;
	FireInterval = 1.5f;
	WeaponRange = 4500.0f;

	GetCollision()->InitSphereRadius(450.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Aetheria/Meshes/SM_TitheCruiser.SM_TitheCruiser"));
	if (Mesh.Succeeded())
	{
		GetHullMesh()->SetStaticMesh(Mesh.Object);
	}
}

void AAetheriaTitheCruiser::OnDamaged(float Amount)
{
	Super::OnDamaged(Amount);

	if (!bTauntDelivered && Hull > 0.0f && Hull < MaxHull * 0.5f)
	{
		bTauntDelivered = true;
		PostComms(E_Speaker::CommanderVance, TEXT("Enjoy the Dreadnought while you can, Key-Bearer. I'm coming for it myself."));
	}
}
