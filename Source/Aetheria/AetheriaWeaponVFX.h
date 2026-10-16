// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaWeaponVFX.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;

/**
 *  A burst of light: weapon impacts, the Prismatic Beam's charge-up and ship explosions.
 *  Additive glow sphere with a hot core and a point light that Lumen picks up.
 *  Expands and fades, or (bGrow) swells toward full size for a charge-up.
 */
UCLASS()
class AAetheriaImpactFlash : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Core;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Halo;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HaloMaterial;

public:

	AAetheriaImpactFlash();

	/** Size: diameter in uu at full size; LightIntensity in candelas */
	void Setup(const FLinearColor& Color, float Size, float Life, float LightIntensity, bool bInGrow = false);

	/** Spawns a flash at Location; returns null without a world */
	static AAetheriaImpactFlash* Spawn(UWorld* World, const FVector& Location, const FLinearColor& Color,
		float Size, float Life, float LightIntensity, bool bGrow = false);

	/** Drops the hot core and dims the halo to Glow: a translucent zone marker (the Silence Pulse telegraph) */
	void SetHaloOnly(float Glow);

	virtual void Tick(float DeltaSeconds) override;

private:

	void Apply(float Alpha);

	float TotalLife = 0.3f;
	float Age = 0.0f;
	float FullSize = 100.0f;
	float BaseLight = 0.0f;
	bool bGrow = false;
	bool bHaloOnly = false;
};

/**
 *  A blaster bolt flying from muzzle to target, then bursting in a small impact flash.
 *  Purely visual; the shot's damage is applied when it is fired.
 */
UCLASS()
class AAetheriaLaserBolt : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Core;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Glow;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GlowMaterial;

public:

	AAetheriaLaserBolt();

	/** Width in uu; the bolt is several times longer than wide */
	void Setup(const FVector& Start, const FVector& End, const FLinearColor& Color, float Width, float Speed);

	static AAetheriaLaserBolt* Spawn(UWorld* World, const FVector& Start, const FVector& End,
		const FLinearColor& Color, float Width, float Speed = 9000.0f);

	virtual void Tick(float DeltaSeconds) override;

private:

	FVector Target = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	FLinearColor BoltColor = FLinearColor::White;
	float BoltSpeed = 9000.0f;
	float BoltWidth = 10.0f;
};
