// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AetheriaCommandPawn.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 *  Overhead tactical camera (SLICE_SPEC.md section 8). Ticks while paused and pans on real time,
 *  so the Captain can look around during a tactical pause. BP_CommandPawn is a Blueprint child.
 */
UCLASS(Blueprintable)
class AAetheriaCommandPawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> Camera;

public:

	AAetheriaCommandPawn();

	/** uu per second at full stick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float PanSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float ZoomStep = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MinArmLength = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MaxArmLength = 14000.0f;

	/** Pans the camera; Value.Y is forward (W/S), Value.X is right (A/D) */
	UFUNCTION(BlueprintCallable, Category="Camera")
	void Pan(FVector2D Value);

	UFUNCTION(BlueprintCallable, Category="Camera")
	void Zoom(float Axis);

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	/** Real seconds since last frame; keeps counting while paused */
	float RealDelta = 0.0f;
	float LastRealTime = 0.0f;
};
