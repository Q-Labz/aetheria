// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaSeverantShips.h"
#include "AetheriaTitheShips.generated.h"

/**
 *  Act VIII Iron Tithe warships (ACT_VIII_IRON_TITHE.md 4.1, 8; WORLD_BIBLE.md 2.2).
 *  Grafted hulls carry a Severant Silence emitter bolted through the armour. While the emitter telegraphs,
 *  the spike glows and is exposed: the ship takes GraftExposedMultiplier damage.
 */
UCLASS(Blueprintable)
class AAetheriaGraftedShip : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaGraftedShip();

	/** Damage taken while the graft telegraphs a pulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graft")
	float GraftExposedMultiplier = 1.5f;

	/** True while the Silence emitter telegraphs (the spike is exposed) */
	UFUNCTION(BlueprintPure, Category="Graft")
	bool IsGraftExposed() const;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};

/** A Tithe cruiser with a Severant graft spike */
UCLASS(Blueprintable)
class AAetheriaGraftedCruiser : public AAetheriaGraftedShip
{
	GENERATED_BODY()

public:

	AAetheriaGraftedCruiser();
};

/**
 *  Tithe Carrack: heavy siege ship with a spinal ram. It closes on the flagship and rams on contact,
 *  then again every RamCooldown seconds while it stays in contact. Kill it before it closes.
 */
UCLASS(Blueprintable)
class AAetheriaTitheCarrack : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaTitheCarrack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ram")
	float RamDamage = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ram")
	float RamCooldown = 8.0f;

	/** Contact distance beyond the two collision radii */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ram")
	float RamReach = 150.0f;

	virtual void Tick(float DeltaSeconds) override;

protected:

	float NextRamTime = 0.0f;
	bool bRamWarned = false;
};

/**
 *  Tithe Collector: tug-like boarding ship with grapple arms, the class meant to take the Dreadnought.
 *  Once in contact with the flagship it grapples on and slows it by SlowFraction until destroyed.
 */
UCLASS(Blueprintable)
class AAetheriaTitheCollector : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaTitheCollector();

	/** Fraction of the flagship's speed each grappled Collector takes away */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float SlowFraction = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
	float GrappleReach = 200.0f;

	virtual void Tick(float DeltaSeconds) override;
	virtual void Die() override;

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Release();

	UPROPERTY(Transient)
	TObjectPtr<AAetheriaShipBase> Grappled;

	float RemovedSpeed = 0.0f;
};

class AAetheriaWritGraftSpike;

/**
 *  Writ of Sector 7: Commander Vance's flagship, a lengthened cruiser with a graft spike as tall as its tower.
 *  At DisableAtHullPct it is disabled, not destroyed (Vance is taken). With both VanceGraftsKnown and
 *  SeverantAgentsCaptured the spike is a separate target: while it stands the Writ takes SpikeArmour damage;
 *  destroying it ends the Writ's pulses and that protection.
 */
UCLASS(Blueprintable)
class AAetheriaWritOfSector7 : public AAetheriaGraftedShip
{
	GENERATED_BODY()

public:

	AAetheriaWritOfSector7();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Writ")
	float DisableAtHullPct = 0.25f;

	/** Damage taken while the exposed graft spike stands */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Writ")
	float SpikeArmour = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Writ")
	TSoftClassPtr<AAetheriaWritGraftSpike> SpikeClass;

	UFUNCTION(BlueprintPure, Category="Writ")
	bool IsDisabled() const { return bDisabled; }

	/** The spike was destroyed */
	void OnSpikeLost();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Die() override;

protected:

	virtual void BeginPlay() override;
	virtual void OnDamaged(float Amount) override;

	UPROPERTY(Transient)
	TObjectPtr<AAetheriaWritGraftSpike> Spike;

	bool bDisabled = false;
	bool bTaunted = false;
};

/** The Writ's exposed graft spike: a static target that rides on the Writ */
UCLASS(Blueprintable)
class AAetheriaWritGraftSpike : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaWritGraftSpike();

	UPROPERTY(Transient)
	TObjectPtr<AAetheriaWritOfSector7> Writ;

	virtual void Die() override;
};

/** A cable pylon of the Graft Forge: shields the crucible while it stands */
UCLASS(Blueprintable)
class AAetheriaForgePylon : public AAetheriaBastionPylon
{
	GENERATED_BODY()

public:

	AAetheriaForgePylon();
};

/** The Graft Forge's crucible: shielded while its cable pylons stand; pulses Silence */
UCLASS(Blueprintable)
class AAetheriaGraftForge : public AAetheriaSeverantBastion
{
	GENERATED_BODY()

public:

	AAetheriaGraftForge();
};
