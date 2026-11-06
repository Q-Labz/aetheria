// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaFlagship.h"
#include "AetheriaAudio.h"
#include "Components/AudioComponent.h"
#include "AetheriaBeamVFX.h"
#include "AetheriaGameInstance.h"
#include "AetheriaWeaponVFX.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaFlagship::AAetheriaFlagship()
{
	DisplayName = FText::FromString(TEXT("The Crystal Dreadnought"));
	Tags.Add(TEXT("Flagship"));

	MaxHull = 4000.0f;
	MaxShield = 2500.0f;	// Tier 4 Crystalline Barrier
	ShieldRegenPerSec = 60.0f;
	MoveSpeed = 450.0f;
	TurnSpeed = 25.0f;
	WeaponRange = 4000.0f;
	WeaponDamage = 35.0f;
	FireInterval = 0.8f;

	GetCollision()->InitSphereRadius(600.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Aetheria/Meshes/SM_CrystalDreadnought.SM_CrystalDreadnought"));
	if (Mesh.Succeeded())
	{
		GetHullMesh()->SetStaticMesh(Mesh.Object);
	}
}

void AAetheriaFlagship::BeginPlay()
{
	UAetheriaGameInstance* GI = GetAetheriaGameInstance();

	if (GI && GI->CrewInjured >= 3)
	{
		FireInterval *= 1.25f;
		PostComms(E_Speaker::ArisThorne, TEXT("Gun crews are undermanned, Captain."));
	}

	Super::BeginPlay();

	if (GI)
	{
		Hull = MaxHull * GI->FlagshipHullPct;
		if (GI->Background == E_Background::CoalitionVanguard)
		{
			BeamDamage *= 1.15f;
		}
	}

	Preview = SpawnBeam(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * BeamLength,
		BeamRadius * 2.0f, FLinearColor(0.1f, 0.3f, 0.4f), 0.0f);
	if (Preview)
	{
		Preview->SetActorHiddenInGame(true);
	}
}

bool AAetheriaFlagship::CanFireBeam() const
{
	return !bBeamCharging && !bIsDead && GetGameTime() >= BeamReadyTime;
}

float AAetheriaFlagship::GetBeamCooldownRemaining() const
{
	return FMath::Max(0.0f, BeamReadyTime - GetGameTime());
}

void AAetheriaFlagship::FireBeam(FVector Target)
{
	if (!CanFireBeam())
	{
		return;
	}

	BeamDir = (Target - GetActorLocation()).GetSafeNormal2D();
	if (BeamDir.IsNearlyZero())
	{
		BeamDir = GetActorForwardVector().GetSafeNormal2D();
	}

	bBeamCharging = true;
	bHasMoveOrder = false;
	AttackTarget = nullptr;
	SetActorRotation(FRotator(0.0f, BeamDir.Rotation().Yaw, 0.0f));
	PostComms(E_Speaker::VexAI, TEXT("Prismatic array charging."));
	// the 1.6 s whine stretched to the charge time
	ChargeAudio = AetheriaAudio::Loop(this, TEXT("S_Beam_Charge"), GetMuzzle(), AetheriaAudio::ERange::Battle, 0.9f,
		FMath::Clamp(1.6f / FMath::Max(BeamChargeTime, 0.1f), 0.5f, 2.0f));

	// charge flare swelling at the emitter; its lifespan pauses with the game like the charge timer
	if (AAetheriaImpactFlash* Flare = AAetheriaImpactFlash::Spawn(GetWorld(), GetMuzzle()->GetComponentLocation(),
		FLinearColor(0.6f, 0.9f, 1.0f), BeamRadius * 2.4f, BeamChargeTime, 300.0f, true))
	{
		Flare->AttachToComponent(GetMuzzle(), FAttachmentTransformRules::KeepWorldTransform);
	}

	// a world timer: halts while paused, and a Chrono Surge's dilation shortens it (intended)
	GetWorldTimerManager().SetTimer(BeamChargeTimer, this, &AAetheriaFlagship::DischargeBeam, BeamChargeTime, false);
}

void AAetheriaFlagship::DischargeBeam()
{
	if (bIsDead)
	{
		return;
	}

	const FVector Start = GetMuzzle()->GetComponentLocation();
	const FVector End = Start + BeamDir * BeamLength;
	ChargeAudio = nullptr;
	AetheriaAudio::PlayAt(this, TEXT("S_Beam_Fire"), Start, AetheriaAudio::ERange::Battle, 0.8f);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params(TEXT("PrismaticBeam"), false, this);
	GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(BeamRadius), Params);

	TSet<AAetheriaShipBase*> Victims;
	for (const FHitResult& Hit : Hits)
	{
		AAetheriaShipBase* Ship = Cast<AAetheriaShipBase>(Hit.GetActor());
		if (Ship && Ship->IsAlive() && IsHostileTo(Ship))
		{
			Victims.Add(Ship);
		}
	}

	int32 Kills = 0;
	for (AAetheriaShipBase* Victim : Victims)
	{
		UGameplayStatics::ApplyDamage(Victim, BeamDamage * DamageMultiplier * ArchitectBoost, nullptr, this, UAetheriaBeamDamage::StaticClass());
		AAetheriaImpactFlash::Spawn(GetWorld(), Victim->GetActorLocation(), FLinearColor(0.85f, 0.95f, 1.0f), 500.0f, 0.6f, 600.0f);
		if (!Victim->IsAlive())
		{
			++Kills;
		}
	}

	// white core with red, green and blue strands either side
	const FVector Right = FVector::CrossProduct(BeamDir, FVector::UpVector).GetSafeNormal();
	SpawnBeam(Start, End, 260.0f, FLinearColor::White, 0.6f);
	SpawnBeam(Start - Right * 60.0f, End - Right * 60.0f, 70.0f, FLinearColor::Red, 0.5f);
	SpawnBeam(Start, End, 70.0f, FLinearColor::Green, 0.5f);
	SpawnBeam(Start + Right * 60.0f, End + Right * 60.0f, 70.0f, FLinearColor::Blue, 0.5f);

	HidePreview();

	BeamShots += 1;
	BeamKills += Kills;
	bBeamCharging = false;
	BeamReadyTime = GetGameTime() + BeamCooldown;

	if (BeamShots == 1)
	{
		if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
		{
			GI->StoryFlags.Add(TEXT("BeamAnomaly"));
			GI->ArchitectExposure += 1;
		}
		PostComms(E_Speaker::VexAI, TEXT("Prismatic output exceeded design specification by twelve percent. Source of the surplus: unregistered. I am investigating."));
	}
}

void AAetheriaFlagship::InterruptBeam()
{
	if (!bBeamCharging)
	{
		return;
	}
	GetWorldTimerManager().ClearTimer(BeamChargeTimer);
	bBeamCharging = false;
	if (ChargeAudio)
	{
		ChargeAudio->FadeOut(0.15f, 0.0f);
		ChargeAudio = nullptr;
	}
	HidePreview();
	PostComms(E_Speaker::VexAI, TEXT("Silence pulse. Prismatic charge lost. The array is still ready, Captain."));
}

void AAetheriaFlagship::ShowPreview(FVector Target)
{
	if (!Preview)
	{
		return;
	}

	FVector Direction = (Target - GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector Start = GetMuzzle()->GetComponentLocation();
	Preview->Setup(Start, Start + Direction * BeamLength, BeamRadius * 2.0f, FLinearColor(0.1f, 0.3f, 0.4f), 0.0f);
	Preview->SetActorHiddenInGame(false);
}

void AAetheriaFlagship::HidePreview()
{
	if (Preview)
	{
		Preview->SetActorHiddenInGame(true);
	}
}
