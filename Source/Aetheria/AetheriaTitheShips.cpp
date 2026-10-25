// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaTitheShips.h"
#include "AetheriaAudio.h"
#include "AetheriaFlagship.h"
#include "AetheriaGameInstance.h"
#include "AetheriaWeaponVFX.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Tithe bolts: the classic green (AetheriaShipBase's EnemyTracer)
	const FLinearColor TitheTracer(0.15f, 1.0f, 0.12f);

	void SetTitheMesh(AAetheriaShipBase* Ship, const TCHAR* Path, float Scale)
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(Path);
		if (Mesh.Succeeded())
		{
			Ship->GetHullMesh()->SetStaticMesh(Mesh.Object);
			Ship->GetHullMesh()->SetRelativeScale3D(FVector(Scale));
		}
	}

	const TCHAR* CruiserMesh = TEXT("/Game/Aetheria/Meshes/SM_TitheCruiser.SM_TitheCruiser");
	const TCHAR* InterceptorMesh = TEXT("/Game/Aetheria/Meshes/SM_TitheInterceptor.SM_TitheInterceptor");
}

// ---- AAetheriaGraftedShip -----------------------------------------------------------------------

AAetheriaGraftedShip::AAetheriaGraftedShip()
{
	Faction = E_Faction::VanceRemnant;
	TracerColor = TitheTracer;
	bTargetsFlagship = true;
	AggroRange = 7000.0f;
	bHasSilenceEmitter = true;
	PulseInterval = 10.0f;
	PulseRadius = 1300.0f;
}

bool AAetheriaGraftedShip::IsGraftExposed() const
{
	return bHasSilenceEmitter && !bIsDead && GetWorldTimerManager().IsTimerActive(ReleaseTimer);
}

float AAetheriaGraftedShip::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsGraftExposed())
	{
		DamageAmount *= GraftExposedMultiplier;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

AAetheriaGraftedCruiser::AAetheriaGraftedCruiser()
{
	DisplayName = FText::FromString(TEXT("Grafted Tithe cruiser"));
	MaxHull = 4000.0f;
	MaxShield = 1500.0f;
	MoveSpeed = 500.0f;
	WeaponDamage = 90.0f;
	FireInterval = 1.5f;
	WeaponRange = 4500.0f;
	GetCollision()->InitSphereRadius(450.0f);
	SetTitheMesh(this, CruiserMesh, 1.0f);
}

// ---- AAetheriaTitheCarrack ----------------------------------------------------------------------

AAetheriaTitheCarrack::AAetheriaTitheCarrack()
{
	DisplayName = FText::FromString(TEXT("Tithe Carrack"));
	Faction = E_Faction::VanceRemnant;
	TracerColor = TitheTracer;
	bTargetsFlagship = true;

	MaxHull = 6000.0f;
	MaxShield = 1000.0f;
	MoveSpeed = 650.0f;
	TurnSpeed = 40.0f;
	WeaponDamage = 60.0f;
	FireInterval = 2.0f;
	// it closes to ram range rather than standing off
	WeaponRange = 900.0f;

	GetCollision()->InitSphereRadius(560.0f);
	SetTitheMesh(this, CruiserMesh, 1.35f);
}

void AAetheriaTitheCarrack::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead || !AttackTarget || !AttackTarget->IsAlive())
	{
		return;
	}
	const float Reach = GetCollision()->GetScaledSphereRadius() + AttackTarget->GetCollision()->GetScaledSphereRadius() + RamReach;
	const float Distance = FVector::Dist2D(GetActorLocation(), AttackTarget->GetActorLocation());

	if (!bRamWarned && Distance < Reach + 2500.0f && Cast<AAetheriaFlagship>(AttackTarget))
	{
		bRamWarned = true;
		PostComms(E_Speaker::TalaVane, TEXT("The Carrack is on a ramming run at the Dreadnought!"));
	}
	if (Distance > Reach || GetGameTime() < NextRamTime)
	{
		return;
	}

	NextRamTime = GetGameTime() + RamCooldown;
	UGameplayStatics::ApplyDamage(AttackTarget, RamDamage * DamageMultiplier, nullptr, this, UDamageType::StaticClass());
	const FVector Contact = (GetActorLocation() + AttackTarget->GetActorLocation()) * 0.5f;
	AAetheriaImpactFlash::Spawn(GetWorld(), Contact, FLinearColor(1.0f, 0.6f, 0.25f), 900.0f, 0.6f, 1200.0f);
	AetheriaAudio::PlayAt(this, TEXT("S_Explosion_Ship"), Contact, AetheriaAudio::ERange::Battle, 0.9f, 0.7f, TEXT("Explosion"));
	PostComms(E_Speaker::JaxMercer, TEXT("Ram strike on the hull! Brace for the next one!"));
}

// ---- AAetheriaTitheCollector --------------------------------------------------------------------

AAetheriaTitheCollector::AAetheriaTitheCollector()
{
	DisplayName = FText::FromString(TEXT("Tithe Collector"));
	Faction = E_Faction::VanceRemnant;
	TracerColor = TitheTracer;
	bTargetsFlagship = true;

	MaxHull = 1400.0f;
	MaxShield = 400.0f;
	MoveSpeed = 1100.0f;
	TurnSpeed = 90.0f;
	WeaponDamage = 10.0f;
	FireInterval = 1.0f;
	WeaponRange = 500.0f;

	GetCollision()->InitSphereRadius(220.0f);
	SetTitheMesh(this, InterceptorMesh, 1.4f);
}

void AAetheriaTitheCollector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead || Grappled)
	{
		if (Grappled && !Grappled->IsAlive())
		{
			Release();
		}
		return;
	}
	AAetheriaFlagship* Flagship = Cast<AAetheriaFlagship>(AttackTarget);
	if (!Flagship || !Flagship->IsAlive())
	{
		return;
	}
	const float Reach = GetCollision()->GetScaledSphereRadius() + Flagship->GetCollision()->GetScaledSphereRadius() + GrappleReach;
	if (FVector::Dist2D(GetActorLocation(), Flagship->GetActorLocation()) > Reach)
	{
		return;
	}

	Grappled = Flagship;
	RemovedSpeed = Flagship->MoveSpeed * SlowFraction;
	Flagship->MoveSpeed -= RemovedSpeed;
	AttachToActor(Flagship, FAttachmentTransformRules::KeepWorldTransform);
	SetActorTickEnabled(false);
	AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation(), FLinearColor(1.0f, 0.3f, 0.15f), 400.0f, 0.4f, 300.0f);
	PostComms(E_Speaker::VexAI, TEXT("A Collector has grappled the Dreadnought's hull. Our speed is reduced until it is destroyed."));
}

void AAetheriaTitheCollector::Release()
{
	if (Grappled)
	{
		Grappled->MoveSpeed += RemovedSpeed;
		Grappled = nullptr;
		RemovedSpeed = 0.0f;
	}
}

void AAetheriaTitheCollector::Die()
{
	if (!bIsDead)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Release();
	}
	Super::Die();
}

void AAetheriaTitheCollector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Release();
	Super::EndPlay(EndPlayReason);
}

// ---- AAetheriaWritOfSector7 ---------------------------------------------------------------------

AAetheriaWritOfSector7::AAetheriaWritOfSector7()
{
	DisplayName = FText::FromString(TEXT("Writ of Sector 7"));
	MaxHull = 12000.0f;
	MaxShield = 3000.0f;
	MoveSpeed = 420.0f;
	TurnSpeed = 35.0f;
	WeaponDamage = 140.0f;
	FireInterval = 1.2f;
	WeaponRange = 5000.0f;
	BoltWidth = 22.0f;
	PulseRadius = 1800.0f;
	PulseInterval = 12.0f;

	GetCollision()->InitSphereRadius(650.0f);
	SetTitheMesh(this, CruiserMesh, 1.6f);

	SpikeClass = TSoftClassPtr<AAetheriaWritGraftSpike>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_WritGraftSpike.BP_WritGraftSpike_C")));
}

void AAetheriaWritOfSector7::BeginPlay()
{
	Super::BeginPlay();

	// the spike is only a readable target when the fleet knows what the grafts are and how they are grown
	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (!GI || !GI->StoryFlags.Contains(TEXT("VanceGraftsKnown")) || !GI->StoryFlags.Contains(TEXT("SeverantAgentsCaptured")))
	{
		return;
	}
	UClass* Class = SpikeClass.LoadSynchronous();
	if (!Class)
	{
		Class = AAetheriaWritGraftSpike::StaticClass();
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector Where = GetActorLocation() + FVector(0.0f, 0.0f, 450.0f);
	Spike = GetWorld()->SpawnActor<AAetheriaWritGraftSpike>(Class, Where, GetActorRotation(), Params);
	if (Spike)
	{
		Spike->Writ = this;
		Spike->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		PostComms(E_Speaker::NixNull, TEXT("That spike on the Writ is the graft's root. Take it out and the Writ loses its armour and its pulse."));
	}
}

void AAetheriaWritOfSector7::OnSpikeLost()
{
	Spike = nullptr;
	bHasSilenceEmitter = false;
	GetWorldTimerManager().ClearTimer(PulseTimer);
	GetWorldTimerManager().ClearTimer(ReleaseTimer);
	if (Telegraph)
	{
		Telegraph->Destroy();
		Telegraph = nullptr;
	}
	PostComms(E_Speaker::NixNull, TEXT("Spike's down! The Writ is just a cruiser now."));
}

float AAetheriaWritOfSector7::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDisabled)
	{
		return 0.0f;
	}
	if (Spike && Spike->IsAlive())
	{
		DamageAmount *= SpikeArmour;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AAetheriaWritOfSector7::OnDamaged(float Amount)
{
	Super::OnDamaged(Amount);

	if (!bTaunted && Hull < MaxHull * 0.6f)
	{
		bTaunted = true;
		PostComms(E_Speaker::CommanderVance, TEXT("I built a berth for that ship, Key-Bearer. I intend to use it."));
	}
	// disabled, not destroyed: hold the hull at the threshold so the base class does not kill the ship
	if (!bDisabled && Hull <= MaxHull * DisableAtHullPct)
	{
		Hull = FMath::Max(1.0f, MaxHull * DisableAtHullPct);
		bDisabled = true;
		Die();
	}
}

void AAetheriaWritOfSector7::Die()
{
	if (bIsDead)
	{
		return;
	}
	if (!bDisabled)
	{
		Super::Die();
		return;
	}

	// the Writ goes dark and drifts: counted as lost for the battle, but it does not explode
	bIsDead = true;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (Telegraph)
	{
		Telegraph->Destroy();
		Telegraph = nullptr;
	}
	if (Spike)
	{
		Spike->Writ = nullptr;
		Spike->Die();
		Spike = nullptr;
	}
	SetActorTickEnabled(false);
	AAetheriaImpactFlash::Spawn(GetWorld(), GetActorLocation(), FLinearColor(1.0f, 0.5f, 0.2f), 700.0f, 0.5f, 600.0f);
	PostComms(E_Speaker::VexAI, TEXT("The Writ of Sector 7 is disabled. Its drive and weapons are dark."));
	PostComms(E_Speaker::CommanderVance, TEXT("Hold fire. Remnant ships, hold fire. ...Key-Bearer, you have my ship."));
	OnShipDestroyed.Broadcast(this);
}

// ---- AAetheriaWritGraftSpike --------------------------------------------------------------------

AAetheriaWritGraftSpike::AAetheriaWritGraftSpike()
{
	DisplayName = FText::FromString(TEXT("Writ graft spike"));
	Faction = E_Faction::VanceRemnant;
	MaxHull = 800.0f;
	MaxShield = 0.0f;
	ShieldRegenPerSec = 0.0f;
	MoveSpeed = 0.0f;
	TurnSpeed = 0.0f;
	WeaponDamage = 0.0f;
	FireInterval = 0.0f;
	GetCollision()->InitSphereRadius(200.0f);
	SetTitheMesh(this, TEXT("/Game/Aetheria/Environments/SM_BastionPylon.SM_BastionPylon"), 0.8f);
}

void AAetheriaWritGraftSpike::Die()
{
	const bool bWasAlive = !bIsDead;
	Super::Die();
	if (bWasAlive && Writ)
	{
		AAetheriaWritOfSector7* Host = Writ;
		Writ = nullptr;
		Host->OnSpikeLost();
	}
}

// ---- The Graft Forge ----------------------------------------------------------------------------

AAetheriaForgePylon::AAetheriaForgePylon()
{
	DisplayName = FText::FromString(TEXT("Forge cable pylon"));
	MaxHull = 1800.0f;
}

AAetheriaGraftForge::AAetheriaGraftForge()
{
	DisplayName = FText::FromString(TEXT("The Graft Forge crucible"));
	MaxHull = 8000.0f;
	bHasSilenceEmitter = true;
	PulseRadius = 2000.0f;
	PulseInterval = 12.0f;
	ShieldedNote = TEXT("The crucible is shielded while the cable pylons stand. Bring down the pylons first.");
	ExposedNote = TEXT("The cables are cut. The crucible is exposed, Captain.");
}
