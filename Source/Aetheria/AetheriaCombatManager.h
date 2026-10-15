// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaTypes.h"
#include "AetheriaCombatManager.generated.h"

class AAetheriaShipBase;
class AAetheriaFlagship;
class AAetheriaPhantomSquadron;
class AAetheriaTacticalController;
class UAetheriaGameInstance;
class UAetheriaAfterAction;

/**
 *  Runs the Lantern Drift engagement (SLICE_SPEC.md section 10): spawns both fleets at tagged
 *  TargetPoints, opens the battle paused, handles the Chrono Surge, and ends in victory or defeat.
 *  BP_CombatManager is a Blueprint child placed at the level origin.
 */
UCLASS(Blueprintable)
class AAetheriaCombatManager : public AActor
{
	GENERATED_BODY()

public:

	AAetheriaCombatManager();

	// ---- Classes spawned (Blueprint children by default) ----

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSoftClassPtr<AAetheriaFlagship> FlagshipClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSoftClassPtr<AAetheriaPhantomSquadron> SquadronClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSoftClassPtr<AAetheriaShipBase> CorsairClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSoftClassPtr<AAetheriaShipBase> CruiserClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSoftClassPtr<AAetheriaShipBase> InterceptorClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSoftClassPtr<UAetheriaAfterAction> AfterActionClass;

	/** Level opened by Return to Command */
	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TSoftObjectPtr<UWorld> CommandLevel;

	// ---- Battle state ----

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	TObjectPtr<AAetheriaFlagship> Flagship;

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	TArray<TObjectPtr<AAetheriaPhantomSquadron>> Squadrons;

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	TArray<TObjectPtr<AAetheriaShipBase>> PlayerShips;

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	int32 AliveEnemies = 0;

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	bool bSurgeActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Battle")
	bool bBattleOver = false;

	// ---- Values restored by Retry Engagement ----

	UPROPERTY(BlueprintReadOnly, Category="Retry")
	float CachedInstability = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Retry")
	float CachedFlagshipHullPct = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Retry")
	int32 CachedPhantomShipsActive = 200;

	// ---- Actions ----

	/** Spends 30 Chrono charge: living fleet ships run at double time and double damage for 6 s */
	UFUNCTION(BlueprintCallable, Category="Battle")
	void StartSurge();

	UFUNCTION(BlueprintCallable, Category="Battle")
	void EndSurge();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ReturnToCommand();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void RetryEngagement();

	/** Phantom vessels lost so far in this battle */
	UFUNCTION(BlueprintPure, Category="Battle")
	int32 GetPhantomLosses() const;

	/** Phantom Fleet strength including this battle's losses */
	UFUNCTION(BlueprintPure, Category="Battle")
	int32 GetPhantomFleetStrength() const;

	/** Posts a line to the HUD comms log */
	UFUNCTION(BlueprintCallable, Category="Battle")
	void Log(E_Speaker Speaker, const FString& Text);

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyLost(AAetheriaShipBase* Ship);

	UFUNCTION()
	void OnPlayerLost(AAetheriaShipBase* Ship);

	UFUNCTION()
	void OnShipMessage(AAetheriaShipBase* Ship, E_Speaker Speaker, const FText& Message);

	void OpenBattle();
	void Victory();
	void Defeat();
	void ShowAfterAction(bool bWon, const FText& Title, const FText& Body);

	/** Spawns a ship at a tagged TargetPoint, wiring its messages before BeginPlay runs */
	AAetheriaShipBase* SpawnShip(UClass* Class, const FTransform& Where, const FText& Name);

	/** TargetPoints carrying Tag, in level order */
	TArray<AActor*> FindSpawnPoints(FName Tag) const;

	UAetheriaGameInstance* GetAetheriaGameInstance() const;
	AAetheriaTacticalController* GetTacticalController() const;

	/** Posts lines held back until the HUD existed */
	void FlushPendingLog();

	UPROPERTY()
	TArray<TObjectPtr<AAetheriaShipBase>> SurgedShips;

	TArray<TPair<E_Speaker, FString>> PendingLog;

	FTimerHandle OpenTimer;
	FTimerHandle SurgeTimer;
};
