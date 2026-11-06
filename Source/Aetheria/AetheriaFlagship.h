// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaShipBase.h"
#include "GameFramework/DamageType.h"
#include "AetheriaFlagship.generated.h"

class UAudioComponent;

/** Damage dealt by the Prismatic Stellar Beam (synced Phantom squadrons lose ships to it, not just sync) */
UCLASS()
class UAetheriaBeamDamage : public UDamageType
{
	GENERATED_BODY()
};

/**
 *  The Crystal Dreadnought and its Prismatic Stellar Beam (SLICE_SPEC.md section 9).
 *  BP_Flagship is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaFlagship : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaFlagship();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float BeamDamage = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float BeamLength = 9000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float BeamRadius = 250.0f;

	/** Seconds between the order and the discharge; stops while paused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float BeamChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float BeamCooldown = 15.0f;

	/** Output surplus from the unregistered source */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float ArchitectBoost = 1.12f;

	UPROPERTY(BlueprintReadOnly, Category="Beam")
	bool bBeamCharging = false;

	/** Game time at which the beam can fire again */
	UPROPERTY(BlueprintReadOnly, Category="Beam")
	float BeamReadyTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Beam")
	TObjectPtr<AAetheriaBeamVFX> Preview;

	UPROPERTY(BlueprintReadOnly, Category="Beam")
	FVector BeamDir = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category="Beam")
	int32 BeamKills = 0;

	UPROPERTY(BlueprintReadOnly, Category="Beam")
	int32 BeamShots = 0;

	UFUNCTION(BlueprintPure, Category="Beam")
	bool CanFireBeam() const;

	/** Seconds until the beam is ready; 0 when ready */
	UFUNCTION(BlueprintPure, Category="Beam")
	float GetBeamCooldownRemaining() const;

	/** Charges, then fires the beam toward Target */
	UFUNCTION(BlueprintCallable, Category="Beam")
	void FireBeam(FVector Target);

	UFUNCTION(BlueprintCallable, Category="Beam")
	void ShowPreview(FVector Target);

	UFUNCTION(BlueprintCallable, Category="Beam")
	void HidePreview();

	/** A Silence Pulse: a charging beam is lost (the cooldown is not spent) */
	UFUNCTION(BlueprintCallable, Category="Beam")
	void InterruptBeam();

protected:

	virtual void BeginPlay() override;
	virtual bool CanManeuver() const override { return !bBeamCharging; }

	void DischargeBeam();

	FTimerHandle BeamChargeTimer;

	/** The charge whine, cut short if a Silence Pulse interrupts it */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ChargeAudio;
};
