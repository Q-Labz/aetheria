// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaShipBase.h"
#include "AetheriaSeverantShips.generated.h"

class AAetheriaImpactFlash;

/**
 *  Severant warships (WORLD_BIBLE.md 2.4, ACT_VII_CINDER_ARCHIVE.md 4.4). Ships with a Silence emitter
 *  telegraph a pale-blue ring for TelegraphTime, then release a Silence Pulse: within PulseRadius it cancels
 *  the Prismatic Stellar Beam's charge (the cooldown is not spent) and ends the Chrono Surge.
 *  The counter is the tactical pause: spot the ring, move the Dreadnought, or kill the emitter first.
 */
UCLASS(Abstract, Blueprintable)
class AAetheriaSeverantShip : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaSeverantShip();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silence")
	bool bHasSilenceEmitter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silence")
	float PulseRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silence")
	float PulseInterval = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silence")
	float TelegraphTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silence")
	FLinearColor PulseColor = FLinearColor(0.35f, 0.58f, 1.0f);

	virtual void Die() override;

protected:

	virtual void BeginPlay() override;

	/** Starts the telegraph ring */
	void BeginPulse();

	/** The pulse itself */
	void ReleasePulse();

	FTimerHandle PulseTimer;
	FTimerHandle ReleaseTimer;

	UPROPERTY(Transient)
	TObjectPtr<AAetheriaImpactFlash> Telegraph;
};

/** Choir-skiff: fast, lightly built Severant raider */
UCLASS(Blueprintable)
class AAetheriaChoirSkiff : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaChoirSkiff();
};

/** Choir-skiff carrying a Silence emitter */
UCLASS(Blueprintable)
class AAetheriaChoirSkiffEmitter : public AAetheriaChoirSkiff
{
	GENERATED_BODY()

public:

	AAetheriaChoirSkiffEmitter();
};

/** Silence Emitter Frigate: the pulse ship of the Severant line */
UCLASS(Blueprintable)
class AAetheriaEmitterFrigate : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaEmitterFrigate();
};

/** The Last Silence: Exarch Ithane's flagship */
UCLASS(Blueprintable)
class AAetheriaLastSilence : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaLastSilence();

protected:

	virtual void OnDamaged(float Amount) override;

	bool bTaunted = false;
};

/** A shield pylon of the Severant Bastion; static */
UCLASS(Blueprintable)
class AAetheriaBastionPylon : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaBastionPylon();
};

/** The Severant Bastion at the Pyre Gate: static, and its core takes no damage while any pylon stands */
UCLASS(Blueprintable)
class AAetheriaSeverantBastion : public AAetheriaSeverantShip
{
	GENERATED_BODY()

public:

	AAetheriaSeverantBastion();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Pylons still standing */
	UFUNCTION(BlueprintPure, Category="Bastion")
	int32 GetLivePylons() const;

protected:

	bool bShieldNoted = false;
	bool bShieldDownNoted = false;
};
