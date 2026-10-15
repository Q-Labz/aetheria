// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaBeamVFX.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;

/**
 *  Energy beam stretched between two points: the Prismatic Beam, its strands and its aiming preview.
 *  A hot core inside a soft additive glow sheath, fading out over its life; wide beams also carry
 *  a light so Lumen lights the hulls around them. BP_BeamVFX is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaBeamVFX : public AActor
{
	GENERATED_BODY()

	/** Hot core (M_Emissive) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Beam;

	/** Soft additive sheath (M_LaserGlow) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Glow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GlowMaterial;

public:

	AAetheriaBeamVFX();

	/** Stretches the beam from Start to End; Width in uu; Life > 0 fades it out and destroys it after that many seconds */
	UFUNCTION(BlueprintCallable, Category="Beam")
	void Setup(FVector Start, FVector End, float Width, FLinearColor Color, float Life);

	/** Beams at least this wide carry a light */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float LightMinWidth = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam")
	float LightIntensity = 400.0f;

	virtual void Tick(float DeltaSeconds) override;

	UStaticMeshComponent* GetBeam() const { return Beam; }

private:

	float TotalLife = 0.0f;
	float Age = 0.0f;
	float BaseWidth = 0.0f;
	float BaseLight = 0.0f;
	float BaseGlow = 0.0f;
};
