// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetheriaTypes.h"
#include "AetheriaTacticalController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class AAetheriaShipBase;
class AAetheriaFlagship;
class AAetheriaPhantomSquadron;
class AAetheriaCombatManager;
class AAetheriaCommandPawn;
class AAetheriaMoveMarker;
class UAetheriaCombatHUD;

/**
 *  Real-time-with-pause fleet command (SLICE_SPEC.md section 8): selection, move/attack orders,
 *  the Prismatic Beam, Chrono Surge and camera, all usable while paused. Orders given during a
 *  tactical pause execute on resume. BP_TacticalController is a Blueprint child.
 */
UCLASS(Blueprintable)
class AAetheriaTacticalController : public APlayerController
{
	GENERATED_BODY()

public:

	AAetheriaTacticalController();

	// ---- Input ----

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> TacticalContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> TogglePauseAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> AimBeamAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ChronoSurgeAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Group1Action;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Group2Action;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Group3Action;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CameraPanAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSoftClassPtr<UAetheriaCombatHUD> HUDClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSoftClassPtr<AAetheriaMoveMarker> MoveMarkerClass;

	/** Lateral spacing between ships given one move order */
	UPROPERTY(EditDefaultsOnly, Category="Orders")
	float FormationSpacing = 900.0f;

	// ---- State ----

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TArray<TObjectPtr<AAetheriaShipBase>> Selected;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TObjectPtr<AAetheriaFlagship> Flagship;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TObjectPtr<AAetheriaPhantomSquadron> Squadron1;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TObjectPtr<AAetheriaPhantomSquadron> Squadron2;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TObjectPtr<UAetheriaCombatHUD> HUD;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	TObjectPtr<AAetheriaCombatManager> Manager;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	bool bTacPaused = false;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	bool bAimingBeam = false;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	bool bBeamQueued = false;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	FVector QueuedBeamTarget = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Tactical")
	bool bSurgeQueued = false;

	// ---- Actions ----

	/** Point on the battle plane (Z = 0) under the mouse cursor */
	UFUNCTION(BlueprintPure, Category="Tactical")
	FVector CursorToPlane() const;

	/** Pauses or resumes the battle; resuming executes queued orders */
	UFUNCTION(BlueprintCallable, Category="Tactical")
	void SetTacPause(bool bPaused);

	UFUNCTION(BlueprintCallable, Category="Tactical")
	void ExecuteQueued();

	UFUNCTION(BlueprintCallable, Category="Tactical")
	void SelectShips(const TArray<AAetheriaShipBase*>& Ships);

	/** Posts a line to the HUD comms log */
	UFUNCTION(BlueprintCallable, Category="Tactical")
	void Log(E_Speaker Speaker, const FString& Text);

	virtual void PlayerTick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void OnSelect();
	void OnCommand();
	void OnTogglePause();
	void OnAimBeam();
	void OnChronoSurge();
	void OnGroup1();
	void OnGroup2();
	void OnGroup3();
	void OnCameraPan(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);

	/** Living ship under the cursor (Visibility trace), or null */
	AAetheriaShipBase* ShipUnderCursor() const;

	void PruneSelection();
};
