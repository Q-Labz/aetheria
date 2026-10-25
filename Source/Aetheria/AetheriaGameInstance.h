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

	// ---- Act VII (ACT_VII_CINDER_ARCHIVE.md) ----

	/** 6 = the slice (Act VI), 7 = The Cinder Archive, 8 = The Iron Tithe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	int32 CurrentAct = 6;

	/** How the Severants see the fleet, -100 .. 100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	int32 SeverantStanding = -20;

	/** Named campaign states: SupplyLine, ArchiveState, ExarchStatus, ArchitectRevelation2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	TMap<FName, FName> States;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	TArray<FS_Shipment> PendingShipments;

	/** Leaps left during which Outpost One sends nothing (Canopy Probe: go dark) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	int32 OutpostDarkLeaps = 0;

	/** Battle (DT_Battles row) the combat level should stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	FName PendingBattle = NAME_None;

	/** Council (DT_Councils row) whose option started PendingBattle; reopened if the battle is lost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Act VII")
	FName PendingBattleCouncil = NAME_None;

	// ---- Data ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> LeapRoutesTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> StarNodesTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> CouncilsTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UDataTable> BattlesTable;

	/** The star chart of the current act */
	UFUNCTION(BlueprintPure, Category="Data")
	UDataTable* GetStarNodes() const;

	UFUNCTION(BlueprintPure, Category="Data")
	UDataTable* GetLeapRoutes() const;

	/** Arrival events of the current data-driven act (DT_NodeEvents_Act<N>); null in the slice */
	UFUNCTION(BlueprintPure, Category="Data")
	UDataTable* GetNodeEvents() const;

	/** A per-act table, /Game/Aetheria/Data/DT_<Kind>_Act<Numeral> (Kind: StarNodes, LeapRoutes, NodeEvents) */
	UDataTable* GetActTable(int32 Act, const TCHAR* Kind) const;

	/** Roman numeral used in act asset and flag names (7 -> "VII") */
	static FString ActNumeral(int32 Act);

	/** "The Cinder Archive", "The Iron Tithe" */
	static FString ActTitle(int32 Act);

	/** Node a data-driven act starts at */
	static FName ActStartNode(int32 Act);

	/** Whether an act's chart has been built (its star-node table exists) */
	UFUNCTION(BlueprintPure, Category="Data")
	bool HasActData(int32 Act) const;

	/** Act-scoped flag name: "Act<Numeral><Suffix>" for the current act (ActVIIIFinaleWon) */
	FName ActFlag(const TCHAR* Suffix) const;

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

	/** Whether a council option's requirements are met (flags, standing, loyalty, states, background) */
	UFUNCTION(BlueprintPure, Category="Council")
	bool IsOptionAvailable(const FS_CouncilOption& Opt) const;

	UFUNCTION(BlueprintPure, Category="Council")
	FName GetState(FName Key) const;

	// ---- Act VII ----

	/** Carries the campaign into a data-driven act at its start node (7 from the slice's end, then 8, ...) */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void StartAct(int32 Act);

	/** Carries the campaign from the slice's end into Act VII at the Ember Relay */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void StartActVII();

	UFUNCTION(BlueprintPure, Category="Act VII")
	bool IsSupplyLineReady() const;

	/** Whether a shipment can be ordered at the current node (one per node, line ready, outpost not dark) */
	UFUNCTION(BlueprintPure, Category="Act VII")
	bool CanOrderShipment() const;

	/** Orders BioGel, Seeds, Refit or Spire from Outpost One over the chosen supply line */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void OrderShipment(FName Kind);

	/** Standing band: Hostile (<= -40), Wary, or Trusted (>= 30) */
	UFUNCTION(BlueprintPure, Category="Act VII")
	FName StandingBand() const;

	/** The cure track for the status line: none, sample, synthesising, ready, delivered */
	UFUNCTION(BlueprintPure, Category="Act VII")
	FString CureStatus() const;

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

	/** Shipment timers and arrivals, the cure's lab time and the outpost blackout, once per leap */
	void AdvanceSupplies(FString& Report);

	void ApplyShipment(FName Kind, FString& Report);
};
