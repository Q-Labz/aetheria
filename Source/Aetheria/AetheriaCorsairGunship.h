// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaShipBase.h"
#include "AetheriaCorsairGunship.generated.h"

/**
 *  Corsair salvage gunship (Outcast Corsair captains only): flak that hits every hostile nearby
 *  (SLICE_SPEC.md section 9). BP_CorsairGunship is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaCorsairGunship : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaCorsairGunship();

	/** Damage to each hostile in range, every FireInterval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flak")
	float FlakDamage = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flak")
	float FlakRadius = 1500.0f;

protected:

	virtual void FireTick() override;
};
