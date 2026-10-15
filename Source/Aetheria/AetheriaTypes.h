// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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
	CommanderVance
};

/** Side a ship fights for */
UENUM(BlueprintType)
enum class E_Faction : uint8
{
	Player,
	VanceRemnant
};

/** What happens on arrival at a star node */
UENUM(BlueprintType)
enum class E_Encounter : uint8
{
	None,
	Combat,
	Council
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
};
