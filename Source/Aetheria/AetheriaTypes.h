// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "AetheriaTypes.generated.h"

/**
 *  Enums and structs for Aetheria's data layer (SLICE_SPEC.md sections 1-3).
 *  Reflected names drop the F prefix, so Blueprints and the AetheriaTools
 *  toolsets see them as S_StarNode, S_Line, etc. (/Script/Aetheria.<Name>).
 */

/** Captain background chosen at captain creation */
UENUM(BlueprintType)
enum class E_Background : uint8
{
	SynodEnvoy,
	GuildEngineer,
	CoalitionVanguard,
	OutcastCorsair
};

/** Who speaks a dialogue line or gives counsel */
UENUM(BlueprintType)
enum class E_Speaker : uint8
{
	Narrator,
	Captain,
	TalaVane,
	JaxMercer,
	ArisThorne,
	NixNull,
	VexAI,
	Lyra,
	Cormaker,
	CoreArchitect,
	CommanderVance,
	WardenKesh,
	ExarchIthane
};

/** Side a ship fights for */
UENUM(BlueprintType)
enum class E_Faction : uint8
{
	Player,
	VanceRemnant,
	Severant
};

/** What happens on arrival at a star node */
UENUM(BlueprintType)
enum class E_Encounter : uint8
{
	None,
	Combat,
	Council,
	Event
};

/** A star on the Stellar Leap chart (DT_StarNodes) */
USTRUCT(BlueprintType)
struct FS_StarNode : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	FName NodeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	FVector2D MapPos = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	E_Encounter Encounter = E_Encounter::None;

	/** Extra instability on leaps into this node (Act VII ash storms); 0 for the slice chart */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Star Node")
	float HazardSurcharge = 0.0f;
};

/** A one-way leap lane between two star nodes (DT_LeapRoutes, row name From_To) */
USTRUCT(BlueprintType)
struct FS_LeapRoute : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Leap Route")
	FName FromNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Leap Route")
	FName ToNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Leap Route")
	float BaseCost = 0.0f;
};

/** One line of dialogue */
USTRUCT(BlueprintType)
struct FS_Line
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Line")
	E_Speaker Speaker = E_Speaker::Narrator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Line")
	FText Text;

	/** Replaces Text for Captains with the given background */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Line")
	TMap<E_Background, FText> BackgroundOverride;

	/** Line is skipped unless this story flag is set; None shows it always */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Line")
	FName RequiredFlag = NAME_None;
};

/** A dialogue sequence (DT_Sequences) */
USTRUCT(BlueprintType)
struct FS_Sequence : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sequence")
	TArray<FS_Line> Lines;
};

/** An officer's advice shown in the council screen */
USTRUCT(BlueprintType)
struct FS_Counsel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Counsel")
	E_Speaker Officer = E_Speaker::Narrator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Counsel")
	FText Advice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Counsel")
	TMap<E_Background, FText> BackgroundOverride;

	/** Counsel is shown only if this story flag is set (None: always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Counsel")
	FName RequiredFlag = NAME_None;

	/** Counsel is hidden if this story flag is set (None: never) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Counsel")
	FName ForbiddenFlag = NAME_None;
};

/** A council choice and its consequences (DT_Council_Relay, DT_Council_Directive) */
USTRUCT(BlueprintType)
struct FS_CouncilOption : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	FText ApprovalHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	bool RequiresBackground = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	E_Background RequiredBackground = E_Background::SynodEnvoy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	int32 LeaningDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	TMap<E_Speaker, int32> LoyaltyDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	float InstabilityDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	float ChronoDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	FName SetFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option")
	TArray<FS_Line> Outcome;

	// ---- Act VII: requirements (an option whose requirements fail is not offered) ----

	/** Every one of these story flags must be set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	TArray<FName> RequiredFlags;

	/** None of these story flags may be set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	TArray<FName> ForbiddenFlags;

	/** Minimum Severant standing (-1000: no requirement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	int32 MinStanding = -1000;

	/** Maximum Severant standing (1000: no requirement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	int32 MaxStanding = 1000;

	/** At least one of these officers must have at least this loyalty (empty: no requirement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	TMap<E_Speaker, int32> MinLoyaltyAny;

	/** Named state that must hold, e.g. SupplyLine=WeaveLane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Requires")
	TMap<FName, FName> RequiredStates;

	// ---- Act VII: effects ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	int32 StandingDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	int32 PursuitDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	int32 ExposureDelta = 0;

	/** Flagship hull change as a fraction (0.2 = +20%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	float HullDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	TArray<FName> SetFlags;

	/** Named campaign states to set, e.g. SupplyLine=ArchConduit, ArchiveState=Preserved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Effects")
	TMap<FName, FName> SetStates;

	/** Council (DT_Councils row) to hold right after the outcome lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Flow")
	FName NextCouncil = NAME_None;

	/** Battle (DT_Battles row) to fight right after the outcome lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council Option|Flow")
	FName StartBattle = NAME_None;
};

/** A council held at a star node (DT_Councils): the situation, the counsel and the options table */
USTRUCT(BlueprintType)
struct FS_CouncilDef : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	E_Speaker SituationSpeaker = E_Speaker::VexAI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	FText Situation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	TArray<FS_Counsel> Counsel;

	/** The options (rows of FS_CouncilOption) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	TSoftObjectPtr<UDataTable> Options;

	/** The council is held only if every one of these flags is set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	TArray<FName> RequiredFlags;

	/** Sequence (DT_Sequences row) played before the council opens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Council")
	FName Intro = NAME_None;
};

/** What happens on arrival at a star node in a data-driven act (DT_NodeEvents_<Act>, row name = node) */
USTRUCT(BlueprintType)
struct FS_NodeEvent : public FTableRowBase
{
	GENERATED_BODY()

	/** Sequence played on the first arrival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Node Event")
	FName ArrivalSequence = NAME_None;

	/** Flags set on the first arrival (e.g. CureSample) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Node Event")
	TArray<FName> ArrivalFlags;

	/** Councils (DT_Councils rows) held here in order; each is held once, when its requirements are met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Node Event")
	TArray<FName> Councils;
};

/** One kind of enemy (or allied) ship in a battle */
USTRUCT(BlueprintType)
struct FS_BattleGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TSoftClassPtr<AActor> ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	int32 Count = 1;

	/** Fights for the player (Kesh's skiffs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	bool bAllied = false;

	/** Battle is won when every ship of this group is destroyed (empty objective: all enemies) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	bool bObjective = false;
};

/** A battle definition (DT_Battles) */
USTRUCT(BlueprintType)
struct FS_Battle : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<FS_BattleGroup> Groups;

	/** Defend battles: seconds to hold; 0 = destroy the objective */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	float HoldSeconds = 0.0f;

	/** Enemies withdraw (battle won) when their total hull falls below this fraction; 0 = fight to the end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	float WithdrawAtHullPct = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<FS_Line> OpeningComms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	TArray<FName> WinFlags;

	/** Sequence played back on the bridge after a victory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Battle")
	FName Debrief = NAME_None;
};

/** A supply shipment on its way from Outpost One */
USTRUCT(BlueprintType)
struct FS_Shipment
{
	GENERATED_BODY()

	/** BioGel, Seeds, Refit or Spire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply")
	FName Kind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply")
	int32 ArrivesAfterLeaps = 0;
};
