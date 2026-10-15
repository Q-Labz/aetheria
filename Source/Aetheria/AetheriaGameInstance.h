// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AetheriaTypes.h"
#include "AetheriaGameInstance.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampaignStateChanged);

/**
 *  Campaign state and rules for the vertical slice (SLICE_SPEC.md section 4).
 *  BP_AetheriaGameInstance is an empty Blueprint child of this class;
 *  variable and function names match the spec so Blueprints and toolsets use them as written.
 */
UCLASS(Blueprintable)
class UAetheriaGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UAetheriaGameInstance();

	// ---- Campaign state ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	FString CaptainName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	E_Background Background = E_Background::SynodEnvoy;

	/** Star-Drive containment stress, 0-100; above 100 the drive ruptures */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	float Instability = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	float ChronoCharge = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	float FlagshipHullPct = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 CrewInjured = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 BioGelDoses = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 SpireRecalibrations = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 PhantomShipsActive = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 PursuitClock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	FName CurrentNode = TEXT("ArchWake");

	/** Route names (From_To) whose lanes have been charted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	TSet<FName> ChartedRoutes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	TSet<FName> StoryFlags;

	/** -100 (Dominion) to 100 (Custodian) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 ArchitectLeaning = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	int32 ArchitectExposure = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	bool FirstRuptureAbsorbed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	TMap<E_Speaker, int32> CrewLoyalty;

	/** Screen the command level opens on: NewGame, PostCombat or StarMap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	FName NextScreen = TEXT("NewGame");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campaign")
	FText LastReport;

	// ---- Data ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> LeapRoutesTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> StarNodesTable;

	// ---- Events ----

	UPROPERTY(BlueprintAssignable, Category="Campaign")
	FOnCampaignStateChanged OnCampaignStateChanged;

	// ---- Stellar Leap ----

	/** CurrentNode_To, the row name of the lane from the current node */
	UFUNCTION(BlueprintPure, Category="Stellar Leap")
	FName RouteName(FName To) const;

	/** Cost of leaping to To; Found is false when no lane exists */
	UFUNCTION(BlueprintPure, Category="Stellar Leap")
	void GetLeapCost(FName To, float& Cost, float& Projected, bool& WillOverflow, bool& Found) const;

	/** Damage a rupture of the given excess would cause; HullLossPct is a fraction of full hull */
	UFUNCTION(BlueprintPure, Category="Stellar Leap")
	void PreviewRupture(float Excess, float& HullLossPct, int32& Injured, int32& Desync) const;

	/** Leaps to To and returns the encounter waiting there (None if no lane) */
	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	E_Encounter ExecuteLeap(FName To);

	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	void ResolveRupture(float Excess);

	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	void ChartRoute(FName To);

	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	void VentChrono();

	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	void HoldStation();

	UFUNCTION(BlueprintCallable, Category="Stellar Leap")
	void SpireRecalibrate();

	// ---- Crew and council ----

	UFUNCTION(BlueprintCallable, Category="Crew")
	void TreatInjured();

	UFUNCTION(BlueprintCallable, Category="Crew")
	void ModifyLoyalty(E_Speaker Who, int32 Delta);

	UFUNCTION(BlueprintCallable, Category="Council")
	void ApplyOption(const FS_CouncilOption& Opt);

	/** Custodian, Dominion or Undetermined */
	UFUNCTION(BlueprintPure, Category="Council")
	FName LeaningState() const;

	// ---- Dialogue ----

	/** The line's text for this Captain, and whether its required flag is set */
	UFUNCTION(BlueprintPure, Category="Dialogue")
	void ResolveLine(const FS_Line& L, FText& Out, bool& Show) const;

	UFUNCTION(BlueprintPure, Category="Dialogue")
	FString BackgroundTitle() const;

	UFUNCTION(BlueprintPure, Category="Dialogue")
	FText SpeakerName(E_Speaker S) const;

	// ---- Campaign lifecycle ----

	UFUNCTION(BlueprintCallable, Category="Campaign")
	void ApplyBackgroundStart();

	UFUNCTION(BlueprintCallable, Category="Campaign")
	void ResetCampaign();

private:

	static constexpr float OverflowThreshold = 100.0f;
};
