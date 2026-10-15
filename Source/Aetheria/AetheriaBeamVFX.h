// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaBeamVFX.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 *  Emissive cylinder stretched between two points: weapon tracers, the Prismatic Beam and its preview.
 *  BP_BeamVFX is a Blueprint child of this class (SLICE_SPEC.md section 9).
 */
UCLASS(Blueprintable)
class AAetheriaBeamVFX : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Beam;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BeamMaterial;

public:

	AAetheriaBeamVFX();

	/** Stretches the beam from Start to End; Width in uu; Life > 0 destroys it after that many seconds */
	UFUNCTION(BlueprintCallable, Category="Beam")
	void Setup(FVector Start, FVector End, float Width, FLinearColor Color, float Life);

	UStaticMeshComponent* GetBeam() const { return Beam; }
};
