// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCorsairGunship.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaCorsairGunship::AAetheriaCorsairGunship()
{
	DisplayName = FText::FromString(TEXT("Corsair Gunship"));

	MaxHull = 1200.0f;
	MaxShield = 300.0f;
	FireInterval = 0.3f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Aetheria/Meshes/SM_CorsairGunship.SM_CorsairGunship"));
	if (Mesh.Succeeded())
	{
		GetHullMesh()->SetStaticMesh(Mesh.Object);
	}
}

void AAetheriaCorsairGunship::FireTick()
{
	if (bIsDead)
	{
		return;
	}

	const FVector Location = GetActorLocation();
	const float RadiusSq = FMath::Square(FlakRadius);

	TArray<AAetheriaShipBase*> Targets;
	for (TActorIterator<AAetheriaShipBase> It(GetWorld()); It; ++It)
	{
		if (It->IsAlive() && IsHostileTo(*It) && FVector::DistSquared2D(Location, It->GetActorLocation()) <= RadiusSq)
		{
			Targets.Add(*It);
		}
	}

	for (AAetheriaShipBase* Target : Targets)
	{
		UGameplayStatics::ApplyDamage(Target, FlakDamage * DamageMultiplier, nullptr, this, UDamageType::StaticClass());
		SpawnBeam(GetMuzzle()->GetComponentLocation(), Target->GetActorLocation(), 15.0f, TracerColor, 0.08f);
	}
}
