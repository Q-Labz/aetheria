// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AetheriaShipBase.h"
#include "AetheriaPhantomSquadron.generated.h"

class UInstancedStaticMeshComponent;

/**
 *  Twenty Phantom battle-cruisers fighting as one unit (SLICE_SPEC.md section 9).
 *  Hull is pooled; every HullPerShip lost removes a ship from the formation and from its firepower.
 *  BP_PhantomSquadron is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaPhantomSquadron : public AAetheriaShipBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInstancedStaticMeshComponent> Formation;

public:

	AAetheriaPhantomSquadron();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Squadron")
	float HullPerShip = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Squadron")
	int32 ShipsInSquadron = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Squadron")
	float DmgPerShip = 3.0f;

	/** Distance between ships in the wedge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Squadron")
	float FormationSpacing = 220.0f;

	/** Ships still flying: Ceil(Hull / HullPerShip) */
	UFUNCTION(BlueprintPure, Category="Squadron")
	int32 GetAliveShips() const;

	virtual FText GetBarLabel() const override;
	virtual void OnConstruction(const FTransform& Transform) override;

protected:

	virtual void BeginPlay() override;
	virtual void FireTick() override;
	virtual void OnDamaged(float Amount) override;

	/** Lays out Count ships in a 5-wide wedge */
	void BuildFormation(int32 Count);
};
