// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaTypes.h"
#include "AetheriaShipBase.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UArrowComponent;
class UWidgetComponent;
class UAetheriaGameInstance;
class AAetheriaBeamVFX;
class UAetheriaShipBar;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipEvent, AAetheriaShipBase*, Ship);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShipMessage, AAetheriaShipBase*, Ship, E_Speaker, Speaker, const FText&, Message);

/**
 *  A ship in tactical combat (SLICE_SPEC.md section 9): movement and attack orders, auto-fire,
 *  shields that regenerate after a lull, damage, death and enemy AI.
 *  Timers and Tick stop while the game is paused, which is what makes tactical pause work.
 *  BP_ShipBase is a Blueprint child; the other ship classes derive from this one.
 */
UCLASS(Blueprintable)
class AAetheriaShipBase : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	/** Selection and weapon hits; blocks Visibility */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> HullMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> SelectionRing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UArrowComponent> Muzzle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> HealthBar;

public:

	AAetheriaShipBase();

	// ---- Identity ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship")
	E_Faction Faction = E_Faction::Player;

	// ---- Stats (first-pass values) ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHull = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxShield = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float ShieldRegenPerSec = 10.0f;

	/** Seconds without being hit before shields regenerate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float ShieldRegenDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MoveSpeed = 900.0f;

	/** Degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float TurnSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float WeaponRange = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float WeaponDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float FireInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float AggroRange = 7000.0f;

	/** Scales outgoing damage (Chrono Surge, matrix strain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float DamageMultiplier = 1.0f;

	/** Enemy AI goes straight for the flagship */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	bool bTargetsFlagship = false;

	// ---- Runtime ----

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	float Hull = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	float Shield = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	FVector MoveTarget = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	bool bHasMoveOrder = false;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	TObjectPtr<AAetheriaShipBase> AttackTarget;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category="Runtime")
	float LastHitTime = -1000.0f;

	/** The BP_CombatManager that spawned this ship */
	UPROPERTY(BlueprintReadWrite, Category="Runtime")
	TObjectPtr<AActor> Manager;

	// ---- Visuals ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	TSoftClassPtr<AAetheriaBeamVFX> BeamVFXClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Visuals")
	TSoftClassPtr<UAetheriaShipBar> ShipBarClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visuals")
	FLinearColor TracerColor = FLinearColor(0.3f, 0.8f, 1.0f);

	/** Blaster bolt width and speed (uu, uu/s); bolts are visual, damage lands when fired */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visuals")
	float BoltWidth = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visuals")
	float BoltSpeed = 9000.0f;

	// ---- Events ----

	UPROPERTY(BlueprintAssignable, Category="Ship")
	FOnShipEvent OnShipDestroyed;

	UPROPERTY(BlueprintAssignable, Category="Ship")
	FOnShipEvent OnShipDamaged;

	/** A crew or comms line for the HUD's comms log */
	UPROPERTY(BlueprintAssignable, Category="Ship")
	FOnShipMessage OnShipMessage;

	// ---- Orders ----

	UFUNCTION(BlueprintCallable, Category="Orders")
	void IssueMove(FVector Location);

	UFUNCTION(BlueprintCallable, Category="Orders")
	void IssueAttack(AAetheriaShipBase* Target);

	UFUNCTION(BlueprintCallable, Category="Orders")
	void SetSelected(bool bSelected);

	// ---- Queries ----

	/** Nearest living ship of another faction within Range, or null */
	UFUNCTION(BlueprintPure, Category="Combat")
	AAetheriaShipBase* FindNearestHostile(float Range) const;

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsHostileTo(const AAetheriaShipBase* Other) const;

	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsAlive() const { return !bIsDead; }

	/** Text the health bar shows */
	UFUNCTION(BlueprintPure, Category="Ship")
	virtual FText GetBarLabel() const;

	/** The living ship tagged Flagship, or null */
	UFUNCTION(BlueprintPure, Category="Combat", meta=(WorldContext="WorldContextObject"))
	static AAetheriaShipBase* FindFlagship(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Combat")
	virtual void Die();

	// ---- AActor ----

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	USphereComponent* GetCollision() const { return Collision; }
	UStaticMeshComponent* GetHullMesh() const { return HullMesh; }
	UArrowComponent* GetMuzzle() const { return Muzzle; }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Auto-fire at AttackTarget if it is in range, else at the nearest hostile in range */
	UFUNCTION()
	virtual void FireTick();

	UFUNCTION()
	void RegenTick();

	/** Enemy orders, once a second */
	UFUNCTION()
	virtual void AIThink();

	/** Called after damage lands, before death is checked */
	virtual void OnDamaged(float Amount) {}

	/** False while the ship must hold still (the flagship charging its beam) */
	virtual bool CanManeuver() const { return true; }

	/** Fires a blaster bolt from the muzzle at Target */
	void FireBolt(const AActor* Target, float Width) const;

	/** Spawns a BP_BeamVFX between two points */
	AAetheriaBeamVFX* SpawnBeam(const FVector& Start, const FVector& End, float Width, const FLinearColor& Color, float Life) const;

	/** Raises OnShipMessage and logs the line */
	void PostComms(E_Speaker Speaker, const FString& Message);

	UAetheriaGameInstance* GetAetheriaGameInstance() const;

	/** Turns toward and moves to Destination; true when within Tolerance */
	bool SteerTowards(const FVector& Destination, float DeltaSeconds, float Tolerance);

	/** Turns toward Point without moving */
	void FaceTowards(const FVector& Point, float DeltaSeconds);

	float GetGameTime() const;

	FTimerHandle FireTimer;
	FTimerHandle RegenTimer;
	FTimerHandle AITimer;
};
