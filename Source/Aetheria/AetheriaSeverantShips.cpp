// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaSeverantShips.h"
#include "AetheriaAudio.h"
#include "AetheriaCombatManager.h"
#include "AetheriaFlagship.h"
#include "AetheriaWeaponVFX.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	void SetMesh(AAetheriaShipBase* Ship, const TCHAR* Path, float Scale = 1.0f)
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(Path);
		if (Mesh.Succeeded())
		{
			Ship->GetHullMesh()->SetStaticMesh(Mesh.Object);
			Ship->GetHullMesh()->SetRelativeScale3D(FVector(Scale));
		}
	}
}

// ---- AAetheriaSeverantShip ------------------------------------------------------------------------

AAetheriaSeverantShip::AAetheriaSeverantShip()
{
	Faction = E_Faction::Severant;
	TracerColor = FLinearColor(0.55f, 0.75f, 1.0f);
	// Severant raiders hunt: they come to the fleet rather than wait at the edge of the field
	AggroRange = 14000.0f;
}

void AAetheriaSeverantShip::BeginPlay()
{
	Super::BeginPlay();

	if (bHasSilenceEmitter)
	{
		// staggered so a group of emitters does not pulse in unison
		const float First = FMath::FRandRange(PulseInterval * 0.4f, PulseInterval);
		GetWorldTimerManager().SetTimer(PulseTimer, FTimerDelegate::CreateUObject(this, &AAetheriaSeverantShip::BeginPulse), PulseInterval, true, First);
	}
}

void AAetheriaSeverantShip::BeginPulse()
{
	if (bIsDead)
	{
		return;
	}
	// the telegraph: a ring swelling to the pulse's reach (timers pause with the game, so the pause reads it)
	Telegraph = AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation(), PulseColor, PulseRadius * 2.0f, TelegraphTime, 200.0f, true);
	if (Telegraph)
	{
		Telegraph->SetHaloOnly(0.6f);
		Telegraph->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
	GetWorldTimerManager().SetTimer(ReleaseTimer, FTimerDelegate::CreateUObject(this, &AAetheriaSeverantShip::ReleasePulse), TelegraphTime, false);
	AetheriaAudio::PlayAt(this, TEXT("S_Pulse_Telegraph"), GetActorLocation(), AetheriaAudio::ERange::Battle, 0.85f,
		FMath::Clamp(1.5f / FMath::Max(TelegraphTime, 0.1f), 0.5f, 2.0f), TEXT("Pulse"));
}

void AAetheriaSeverantShip::ReleasePulse()
{
	if (bIsDead)
	{
		return;
	}
	AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation(), FLinearColor(0.8f, 0.9f, 1.0f), PulseRadius * 2.2f, 0.35f, 900.0f);
	AetheriaAudio::PlayAt(this, TEXT("S_Pulse_Release"), GetActorLocation(), AetheriaAudio::ERange::Battle, 1.0f, 1.0f, TEXT("Pulse"));

	const float RadiusSq = FMath::Square(PulseRadius);
	bool bCaughtPlayer = false;
	for (TActorIterator<AAetheriaShipBase> It(GetWorld()); It; ++It)
	{
		if (!It->IsAlive() || !IsHostileTo(*It) || FVector::DistSquared2D(It->GetActorLocation(), GetActorLocation()) > RadiusSq)
		{
			continue;
		}
		bCaughtPlayer = true;
		if (AAetheriaFlagship* Flagship = Cast<AAetheriaFlagship>(*It))
		{
			Flagship->InterruptBeam();
		}
	}

	if (bCaughtPlayer)
	{
		if (AAetheriaCombatManager* CombatManager = Cast<AAetheriaCombatManager>(Manager))
		{
			if (CombatManager->bSurgeActive)
			{
				CombatManager->EndSurge();
				PostComms(E_Speaker::JaxMercer, TEXT("Silence pulse! The Chrono-Core surge just died on us."));
			}
		}
	}
}

void AAetheriaSeverantShip::Die()
{
	GetWorldTimerManager().ClearTimer(PulseTimer);
	GetWorldTimerManager().ClearTimer(ReleaseTimer);
	if (Telegraph)
	{
		Telegraph->Destroy();
		Telegraph = nullptr;
	}
	Super::Die();
}

// ---- Ship classes -----------------------------------------------------------------------------

AAetheriaChoirSkiff::AAetheriaChoirSkiff()
{
	DisplayName = FText::FromString(TEXT("Choir-skiff"));
	MaxHull = 320.0f;
	MaxShield = 120.0f;
	MoveSpeed = 1350.0f;
	TurnSpeed = 110.0f;
	WeaponDamage = 15.0f;
	FireInterval = 0.6f;
	WeaponRange = 3000.0f;
	BoltWidth = 9.0f;
	GetCollision()->InitSphereRadius(140.0f);
	SetMesh(this, TEXT("/Game/Aetheria/Meshes/SM_SeverantChoirSkiff.SM_SeverantChoirSkiff"));
}

AAetheriaChoirSkiffEmitter::AAetheriaChoirSkiffEmitter()
{
	DisplayName = FText::FromString(TEXT("Choir-skiff (Silence emitter)"));
	bHasSilenceEmitter = true;
	PulseInterval = 11.0f;
	PulseRadius = 1300.0f;
}

AAetheriaEmitterFrigate::AAetheriaEmitterFrigate()
{
	DisplayName = FText::FromString(TEXT("Silence Emitter Frigate"));
	bHasSilenceEmitter = true;
	PulseInterval = 8.0f;
	MaxHull = 1500.0f;
	MaxShield = 500.0f;
	MoveSpeed = 600.0f;
	TurnSpeed = 45.0f;
	WeaponDamage = 35.0f;
	FireInterval = 1.2f;
	WeaponRange = 3800.0f;
	BoltWidth = 14.0f;
	GetCollision()->InitSphereRadius(300.0f);
	SetMesh(this, TEXT("/Game/Aetheria/Meshes/SM_SilenceEmitterFrigate.SM_SilenceEmitterFrigate"));
}

AAetheriaLastSilence::AAetheriaLastSilence()
{
	DisplayName = FText::FromString(TEXT("The Last Silence"));
	bHasSilenceEmitter = true;
	bTargetsFlagship = true;
	PulseInterval = 10.0f;
	PulseRadius = 2200.0f;
	MaxHull = 7000.0f;
	MaxShield = 2500.0f;
	ShieldRegenPerSec = 40.0f;
	MoveSpeed = 260.0f;
	TurnSpeed = 18.0f;
	WeaponDamage = 120.0f;
	FireInterval = 1.4f;
	WeaponRange = 5000.0f;
	BoltWidth = 24.0f;
	GetCollision()->InitSphereRadius(700.0f);
	SetMesh(this, TEXT("/Game/Aetheria/Meshes/SM_TheLastSilence.SM_TheLastSilence"));
}

void AAetheriaLastSilence::OnDamaged(float Amount)
{
	Super::OnDamaged(Amount);
	if (!bTaunted && Hull > 0.0f && Hull < MaxHull * 0.5f)
	{
		bTaunted = true;
		PostComms(E_Speaker::ExarchIthane, TEXT("Break me, then. The Silence will outlive us both."));
	}
}

AAetheriaBastionPylon::AAetheriaBastionPylon()
{
	DisplayName = FText::FromString(TEXT("Bastion shield pylon"));
	MaxHull = 1600.0f;
	MaxShield = 600.0f;
	MoveSpeed = 0.0f;
	TurnSpeed = 0.0f;
	WeaponDamage = 30.0f;
	FireInterval = 1.0f;
	WeaponRange = 3200.0f;
	GetCollision()->InitSphereRadius(260.0f);
	SetMesh(this, TEXT("/Game/Aetheria/Environments/SM_BastionPylon.SM_BastionPylon"), 2.0f);
}

AAetheriaSeverantBastion::AAetheriaSeverantBastion()
{
	DisplayName = FText::FromString(TEXT("The Severant Bastion"));
	MaxHull = 9000.0f;
	MaxShield = 0.0f;
	MoveSpeed = 0.0f;
	TurnSpeed = 0.0f;
	WeaponDamage = 90.0f;
	FireInterval = 1.6f;
	WeaponRange = 5200.0f;
	BoltWidth = 22.0f;
	GetCollision()->InitSphereRadius(800.0f);
	SetMesh(this, TEXT("/Game/Aetheria/Environments/SM_SeverantBastion.SM_SeverantBastion"), 2.0f);
}

int32 AAetheriaSeverantBastion::GetLivePylons() const
{
	int32 Count = 0;
	for (TActorIterator<AAetheriaBastionPylon> It(GetWorld()); It; ++It)
	{
		Count += It->IsAlive() ? 1 : 0;
	}
	return Count;
}

float AAetheriaSeverantBastion::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLivePylons() > 0)
	{
		if (!bShieldNoted)
		{
			bShieldNoted = true;
			PostComms(E_Speaker::VexAI, TEXT("The Bastion core is shielded while its pylons stand. Bring down the pylons first."));
		}
		AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation() + FMath::VRand() * 500.0f, PulseColor, 300.0f, 0.3f, 80.0f);
		return 0.0f;
	}
	if (!bShieldDownNoted)
	{
		bShieldDownNoted = true;
		PostComms(E_Speaker::VexAI, TEXT("Pylons down. The core is exposed, Captain."));
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
