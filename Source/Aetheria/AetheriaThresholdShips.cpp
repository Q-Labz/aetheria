// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaThresholdShips.h"
#include "AetheriaAudio.h"
#include "AetheriaBeamVFX.h"
#include "AetheriaFlagship.h"
#include "AetheriaWeaponVFX.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// the Listener's violet, and the Answerers' pale crystal
	const FLinearColor ListenerTracer(0.72f, 0.32f, 1.0f);
	const FLinearColor AnswererTracer(0.70f, 0.95f, 1.0f);

	void SetThresholdMesh(AAetheriaShipBase* Ship, const TCHAR* Path, float Scale)
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(Path);
		if (Mesh.Succeeded())
		{
			Ship->GetHullMesh()->SetStaticMesh(Mesh.Object);
			Ship->GetHullMesh()->SetRelativeScale3D(FVector(Scale));
		}
	}

	const TCHAR* ThresholdPhantomMesh = TEXT("/Game/Aetheria/Meshes/SM_PhantomCruiser.SM_PhantomCruiser");
	const TCHAR* ThresholdPylonMesh = TEXT("/Game/Aetheria/Environments/SM_BastionPylon.SM_BastionPylon");

	// Vex explains the firing line once per battle, not once per sentinel
	TWeakObjectPtr<UWorld> LanceWarnedWorld;
}

// ---- AAetheriaSyncedSquadron --------------------------------------------------------------------

AAetheriaSyncedSquadron::AAetheriaSyncedSquadron()
{
	DisplayName = FText::FromString(TEXT("Squadron III (synced)"));
	Faction = E_Faction::Listener;
	TracerColor = ListenerTracer;
	bTargetsFlagship = false;
	AggroRange = 12000.0f;
	// the sync is harder to break than the hulls are to hole
	HullPerShip = 260.0f;
	MaxShield = 600.0f;
}

void AAetheriaSyncedSquadron::BeginPlay()
{
	// not a fleet squadron: none of the fleet's matrix strain or Nix's response bonus applies to it
	MaxHull = HullPerShip * ShipsInSquadron;
	AAetheriaShipBase::BeginPlay();
	BuildFormation(ShipsInSquadron);
}

FText AAetheriaSyncedSquadron::GetBarLabel() const
{
	const int32 Sync = MaxHull > 0.0f ? FMath::RoundToInt(100.0f * Hull / MaxHull) : 0;
	return FText::FromString(FString::Printf(TEXT("%s · sync %d%% · %d/%d"), *DisplayName.ToString(), Sync,
		ShipsInSquadron - ShipsLost, ShipsInSquadron));
}

float AAetheriaSyncedSquadron::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}
	// the beam does not break the sync gently: it destroys hulls
	if (DamageEvent.DamageTypeClass && DamageEvent.DamageTypeClass->IsChildOf(UAetheriaBeamDamage::StaticClass()))
	{
		ShipsLost = FMath::Min(ShipsInSquadron, ShipsLost + BeamShipLoss);
		BuildFormation(ShipsInSquadron - ShipsLost);
		if (!bBeamWarned)
		{
			bBeamWarned = true;
			PostComms(E_Speaker::NixNull, FString::Printf(TEXT("The beam just killed %d of them, Captain. Those are our ships. Wear the sync down with the guns."), BeamShipLoss));
		}
		if (ShipsLost >= ShipsInSquadron)
		{
			Super::Die();
			return DamageAmount;
		}
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AAetheriaSyncedSquadron::OnDamaged(float Amount)
{
	// the hull is the sync: ships do not fall out of the formation as it weakens
	AAetheriaShipBase::OnDamaged(Amount);
}

void AAetheriaSyncedSquadron::FireTick()
{
	// firepower follows the hulls still flying and how strongly the sync holds them
	const float SyncFraction = MaxHull > 0.0f ? Hull / MaxHull : 0.0f;
	WeaponDamage = DmgPerShip * (ShipsInSquadron - ShipsLost) * (0.5f + 0.5f * SyncFraction);
	AAetheriaShipBase::FireTick();
}

void AAetheriaSyncedSquadron::Die()
{
	if (bIsDead)
	{
		return;
	}
	if (ShipsLost >= ShipsInSquadron)
	{
		Super::Die();
		return;
	}

	// disabled, not destroyed: the formation goes dark and drifts, counted as down for the battle
	bDisabled = true;
	bIsDead = true;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	SetActorTickEnabled(false);
	AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation(), TracerColor, 1400.0f, 0.8f, 800.0f);
	PostComms(DisabledSpeaker, DisabledLine);
	OnShipDestroyed.Broadcast(this);
}

AAetheriaHeldSquadron::AAetheriaHeldSquadron()
{
	DisplayName = FText::FromString(TEXT("Held Phantom squadron"));
	Faction = E_Faction::Answerer;
	TracerColor = AnswererTracer;
	bReclaimAddsToFleet = false;
	DisabledSpeaker = E_Speaker::CoreArchitect;
	DisabledLine = TEXT("I release this squadron to you, Key-Bearer. I have recorded that you made me.");
}

// ---- AAetheriaEchoHull --------------------------------------------------------------------------

AAetheriaEchoHull::AAetheriaEchoHull()
{
	DisplayName = FText::FromString(TEXT("Echo hull"));
	Faction = E_Faction::Listener;
	TracerColor = ListenerTracer;

	MaxHull = 700.0f;
	MaxShield = 0.0f;
	ShieldRegenPerSec = 0.0f;
	MoveSpeed = 420.0f;
	TurnSpeed = 45.0f;
	WeaponDamage = 0.0f;
	FireInterval = 0.0f;
	// it closes to contact instead of standing off
	WeaponRange = 200.0f;
	AggroRange = 16000.0f;

	GetCollision()->InitSphereRadius(260.0f);
	SetThresholdMesh(this, ThresholdPhantomMesh, 1.8f);
}

void AAetheriaEchoHull::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead || !AttackTarget || !AttackTarget->IsAlive())
	{
		return;
	}
	const float Reach = GetCollision()->GetScaledSphereRadius() + AttackTarget->GetCollision()->GetScaledSphereRadius() + BurstReach;
	if (FVector::Dist2D(GetActorLocation(), AttackTarget->GetActorLocation()) <= Reach)
	{
		Burst();
	}
}

void AAetheriaEchoHull::Burst()
{
	const FVector Location = GetActorLocation();
	for (TActorIterator<AAetheriaShipBase> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->IsAlive() && IsHostileTo(*It) && FVector::Dist2D(Location, It->GetActorLocation()) <= BurstRadius)
		{
			UGameplayStatics::ApplyDamage(*It, BurstDamage * DamageMultiplier, nullptr, this, UDamageType::StaticClass());
		}
	}
	AAetheriaImpactFlash::Spawn(GetWorld(), Location, ListenerTracer, BurstRadius * 2.0f, 0.7f, 1500.0f);
	Die();
}

// ---- AAetheriaYardSentinel ----------------------------------------------------------------------

AAetheriaYardSentinel::AAetheriaYardSentinel()
{
	DisplayName = FText::FromString(TEXT("Yard sentinel"));
	Faction = E_Faction::Answerer;
	TracerColor = AnswererTracer;

	MaxHull = 5000.0f;
	MaxShield = 2000.0f;
	ShieldRegenPerSec = 40.0f;
	MoveSpeed = 0.0f;
	TurnSpeed = 0.0f;
	// point defence between lances
	WeaponDamage = 25.0f;
	FireInterval = 1.0f;
	WeaponRange = 3000.0f;

	GetCollision()->InitSphereRadius(420.0f);
	SetThresholdMesh(this, ThresholdPylonMesh, 1.3f);
}

void AAetheriaYardSentinel::BeginPlay()
{
	Super::BeginPlay();

	// staggered so the ring's lances do not fire in unison
	const float First = FMath::FRandRange(LanceInterval * 0.5f, LanceInterval);
	GetWorldTimerManager().SetTimer(LanceTimer, FTimerDelegate::CreateUObject(this, &AAetheriaYardSentinel::BeginLance), LanceInterval, true, First);
}

void AAetheriaYardSentinel::BeginLance()
{
	if (bIsDead)
	{
		return;
	}
	AAetheriaShipBase* Target = FindFlagship(this);
	if (!Target || FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) > LanceLength)
	{
		Target = FindNearestHostile(LanceLength);
	}
	if (!Target)
	{
		return;
	}

	// the line is locked now: whatever stands on it in TelegraphTime is hit
	const FVector Start = GetActorLocation();
	LanceDir = (Target->GetActorLocation() - Start).GetSafeNormal2D();
	Telegraph = SpawnBeam(Start, Start + LanceDir * LanceLength, 24.0f, LanceColor, TelegraphTime);
	if (LanceWarnedWorld.Get() != GetWorld())
	{
		LanceWarnedWorld = GetWorld();
		PostComms(E_Speaker::VexAI, TEXT("A yard sentinel has locked a firing line. Anything on that line when it fires will be hit."));
	}
	GetWorldTimerManager().SetTimer(ReleaseTimer, FTimerDelegate::CreateUObject(this, &AAetheriaYardSentinel::ReleaseLance), TelegraphTime, false);
}

void AAetheriaYardSentinel::ReleaseLance()
{
	Telegraph = nullptr;
	if (bIsDead)
	{
		return;
	}
	const FVector Start = GetActorLocation();
	const FVector End = Start + LanceDir * LanceLength;

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params(TEXT("YardLance"), false, this);
	GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(LanceRadius), Params);

	TSet<AAetheriaShipBase*> Victims;
	for (const FHitResult& Hit : Hits)
	{
		AAetheriaShipBase* Ship = Cast<AAetheriaShipBase>(Hit.GetActor());
		if (Ship && Ship->IsAlive() && IsHostileTo(Ship))
		{
			Victims.Add(Ship);
		}
	}
	for (AAetheriaShipBase* Victim : Victims)
	{
		UGameplayStatics::ApplyDamage(Victim, LanceDamage * DamageMultiplier, nullptr, this, UDamageType::StaticClass());
		AAetheriaImpactFlash::Spawn(GetWorld(), Victim->GetActorLocation(), LanceColor, 600.0f, 0.5f, 700.0f);
	}
	SpawnBeam(Start, End, 160.0f, LanceColor, 0.5f);
	AetheriaAudio::PlayAt(this, TEXT("S_Beam_Fire"), Start, AetheriaAudio::ERange::Battle, 0.5f, 1.3f);
}

void AAetheriaYardSentinel::Die()
{
	if (Telegraph)
	{
		Telegraph->Destroy();
		Telegraph = nullptr;
	}
	Super::Die();
}

// ---- AAetheriaSentinelDrone ---------------------------------------------------------------------

AAetheriaSentinelDrone::AAetheriaSentinelDrone()
{
	DisplayName = FText::FromString(TEXT("Sentinel drone"));
	Faction = E_Faction::Answerer;
	TracerColor = AnswererTracer;

	MaxHull = 220.0f;
	MaxShield = 60.0f;
	MoveSpeed = 1600.0f;
	TurnSpeed = 180.0f;
	WeaponDamage = 4.0f;
	FireInterval = 0.6f;
	WeaponRange = 1500.0f;
	AggroRange = 14000.0f;
	BoltWidth = 8.0f;

	GetCollision()->InitSphereRadius(120.0f);
	SetThresholdMesh(this, ThresholdPhantomMesh, 0.45f);
}
