// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaShipBase.h"
#include "AetheriaTitheCruiser.generated.h"

/**
 *  Picket Cruiser 'Tithe of Sector 7', Commander Vance's flagship hunter (SLICE_SPEC.md section 9).
 *  BP_TitheCruiser is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaTitheCruiser : public AAetheriaShipBase
{
	GENERATED_BODY()

public:

	AAetheriaTitheCruiser();

	/** Vance's taunt has played (first time below half hull) */
	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	bool bTauntDelivered = false;

protected:

	virtual void OnDamaged(float Amount) override;
};
