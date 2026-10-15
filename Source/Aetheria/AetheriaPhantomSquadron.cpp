// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaPhantomSquadron.h"
#include "AetheriaGameInstance.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaPhantomSquadron::AAetheriaPhantomSquadron()
{
	DisplayName = FText::FromString(TEXT("Phantom Squadron"));

	MoveSpeed = 1000.0f;
	TurnSpeed = 70.0f;
	MaxShield = 400.0f;
	MaxHull = HullPerShip * ShipsInSquadron;

	GetCollision()->InitSphereRadius(600.0f);

	Formation = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Formation"));
	Formation->SetupAttachment(GetCollision());
	Formation->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Aetheria/Meshes/SM_PhantomCruiser.SM_PhantomCruiser"));
	if (Mesh.Succeeded())
	{
		Formation->SetStaticMesh(Mesh.Object);
	}
}

void AAetheriaPhantomSquadron::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildFormation(ShipsInSquadron);
}

void AAetheriaPhantomSquadron::BeginPlay()
{
	MaxHull = HullPerShip * ShipsInSquadron;

	if (const UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		if (GI->Instability >= 80.0f)
		{
			DamageMultiplier = 0.8f;
			PostComms(E_Speaker::NixNull, TEXT("Matrix strain. Phantom response degraded."));
		}
		if (GI->CrewLoyalty.FindRef(E_Speaker::NixNull) >= 60)
		{
			DamageMultiplier += 0.1f;
		}
	}

	Super::BeginPlay();
	BuildFormation(GetAliveShips());
}

int32 AAetheriaPhantomSquadron::GetAliveShips() const
{
	if (HullPerShip <= 0.0f)
	{
		return 0;
	}
	return FMath::Clamp(FMath::CeilToInt(Hull / HullPerShip), 0, ShipsInSquadron);
}

FText AAetheriaPhantomSquadron::GetBarLabel() const
{
	return FText::FromString(FString::Printf(TEXT("%s %d/%d"), *DisplayName.ToString(), GetAliveShips(), ShipsInSquadron));
}

void AAetheriaPhantomSquadron::FireTick()
{
	WeaponDamage = DmgPerShip * GetAliveShips();
	Super::FireTick();
}

void AAetheriaPhantomSquadron::OnDamaged(float Amount)
{
	Super::OnDamaged(Amount);

	const int32 Alive = GetAliveShips();
	while (Formation->GetInstanceCount() > Alive)
	{
		Formation->RemoveInstance(Formation->GetInstanceCount() - 1);
	}
}

void AAetheriaPhantomSquadron::BuildFormation(int32 Count)
{
	Formation->ClearInstances();

	// 5 x 4 wedge: rows trail behind the lead row, outer columns sweep further back
	constexpr int32 Columns = 5;
	const float Rows = FMath::DivideAndRoundUp(ShipsInSquadron, Columns);
	const float CentreOffset = (Rows - 1) * FormationSpacing * 0.5f;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const int32 Row = Index / Columns;
		const int32 Column = Index % Columns - Columns / 2;
		const FVector Offset(CentreOffset - Row * FormationSpacing - FMath::Abs(Column) * FormationSpacing * 0.5f,
			Column * FormationSpacing, 0.0f);
		Formation->AddInstance(FTransform(Offset));
	}
}
