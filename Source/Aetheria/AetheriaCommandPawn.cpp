// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCommandPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"

AAetheriaCommandPawn::AAetheriaCommandPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	SpringArm->TargetArmLength = 7000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetFieldOfView(55.0f);
}

void AAetheriaCommandPawn::BeginPlay()
{
	Super::BeginPlay();
	LastRealTime = GetWorld()->GetRealTimeSeconds();

	FPostProcessSettings& Settings = Camera->PostProcessSettings;
	Settings.bOverride_DepthOfFieldFstop = bFocusBattlePlane;
	Settings.bOverride_DepthOfFieldSensorWidth = bFocusBattlePlane;
	Settings.bOverride_DepthOfFieldFocalDistance = bFocusBattlePlane;
	Settings.DepthOfFieldFstop = FocusFstop;
	Settings.DepthOfFieldSensorWidth = FocusSensorWidth;
}

void AAetheriaCommandPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Now = GetWorld()->GetRealTimeSeconds();
	RealDelta = Now - LastRealTime;
	LastRealTime = Now;

	if (bFocusBattlePlane)
	{
		// the pawn sits on the battle plane, so the camera's distance to it is the focus distance
		Camera->PostProcessSettings.DepthOfFieldFocalDistance = FVector::Dist(Camera->GetComponentLocation(), GetActorLocation());
	}
}

void AAetheriaCommandPawn::Pan(FVector2D Value)
{
	// the camera looks down +X, so forward input moves along X and right input along Y
	AddActorWorldOffset(FVector(Value.Y, Value.X, 0.0f) * PanSpeed * RealDelta);
}

void AAetheriaCommandPawn::Zoom(float Axis)
{
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength - Axis * ZoomStep, MinArmLength, MaxArmLength);
}
