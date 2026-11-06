// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaPhantomSquadron.h"
#include "AetheriaThresholdShips.generated.h"

/**
 *  Acts IX-X: what contests the Automated Fleet Key (ACT_IX_THRESHOLD.md 8, ACT_X_GATE_ZERO.md 7).
 *
 *  A synced squadron is twenty Phantom battle-cruisers of the fleet's own 200, held in the Listener's sync.
 *  Its hull is the sync, not the ships: ordinary fire wears the sync down, and at zero the squadron is
 *  disabled and its ships come back to the Key. The Prismatic Stellar Beam is different: every beam hit
 *  destroys BeamShipLoss hulls for good. The tactical choice is not to turn the fleet's big gun on its own ships.
 */
UCLASS(Blueprintable)
class AAetheriaSyncedSquadron : public AAetheriaPhantomSquadron
{
	GENERATED_BODY()

public:

	AAetheriaSyncedSquadron();

	/** Squadron III: disabled ships return to the Phantom Fleet. Held squadrons were never out of it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sync")
	bool bReclaimAddsToFleet = true;

	/** Hulls destroyed by each Prismatic Stellar Beam hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sync")
	int32 BeamShipLoss = 4;

	/** Hulls destroyed so far (by the beam) */
	UPROPERTY(BlueprintReadOnly, Category="Sync")
	int32 ShipsLost = 0;

	/** Who speaks when the sync breaks, and the line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sync")
	E_Speaker DisabledSpeaker = E_Speaker::VexAI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sync")
	FString DisabledLine = TEXT("Squadron III's sync is broken. Its vessels answer to the Key again, Captain.");

	UFUNCTION(BlueprintPure, Category="Sync")
	bool IsDisabled() const { return bDisabled; }

	/** Ships that come back to the Key: the squadron less the destroyed, once it is disabled */
	UFUNCTION(BlueprintPure, Category="Sync")
	int32 GetReclaimedShips() const { return bDisabled ? FMath::Max(0, ShipsInSquadron - ShipsLost) : 0; }

	virtual FText GetBarLabel() const override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Die() override;

protected:

	virtual void BeginPlay() override;
	virtual void FireTick() override;
	virtual void OnDamaged(float Amount) override;

	bool bDisabled = false;
	bool bBeamWarned = false;
};

/** A Phantom squadron the Core Architect holds against the Key-Bearer (Dominion): its losses come out of the fleet */
UCLASS(Blueprintable)
class AAetheriaHeldSquadron : public AAetheriaSyncedSquadron
{
	GENERATED_BODY()

public:

	AAetheriaHeldSquadron();
};

/**
 *  Echo hull: an empty Answerer hull from the Forge Echo, moved by the Listener. It carries no guns; it drifts
 *  at the nearest fleet ship and its dead drive core bursts on contact, damaging everything near it.
 *  Not one of the 214: destroying echo hulls costs the fleet nothing.
 */
UCLASS(Blueprintable)
class AAetheriaEchoHull : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaEchoHull();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Echo")
	float BurstDamage = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Echo")
	float BurstRadius = 700.0f;

	/** Contact distance beyond the two collision radii */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Echo")
	float BurstReach = 120.0f;

	virtual void Tick(float DeltaSeconds) override;

protected:

	void Burst();
};

/**
 *  Yard sentinel: a static Answerer lance on the Meridian yard ring. Every LanceInterval it locks a line on its
 *  target, telegraphs it for TelegraphTime, then fires the lance down that line: every fleet ship on it is hit.
 *  The counter is the tactical pause: see the line, move off it.
 */
UCLASS(Blueprintable)
class AAetheriaYardSentinel : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaYardSentinel();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	float LanceDamage = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	float LanceLength = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	float LanceRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	float LanceInterval = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	float TelegraphTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lance")
	FLinearColor LanceColor = FLinearColor(0.75f, 0.95f, 1.0f);

	virtual void Die() override;

protected:

	virtual void BeginPlay() override;
	virtual void AIThink() override {}

	/** Locks the line and shows it */
	void BeginLance();

	/** Fires down the locked line */
	void ReleaseLance();

	FTimerHandle LanceTimer;
	FTimerHandle ReleaseTimer;
	FVector LanceDir = FVector::ForwardVector;

	UPROPERTY(Transient)
	TObjectPtr<AAetheriaBeamVFX> Telegraph;
};

/** Sentinel drone: a fast, fragile Answerer interceptor; the yard and the root wake them in swarms */
UCLASS(Blueprintable)
class AAetheriaSentinelDrone : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaSentinelDrone();
};
