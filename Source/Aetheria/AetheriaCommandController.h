// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetheriaTypes.h"
#include "AetheriaCommandController.generated.h"

class UDataTable;
class UUserWidget;
class UAetheriaGameInstance;
class UAetheriaCaptainCreation;
class UAetheriaDialogue;
class UAetheriaStarMap;
class UAetheriaCouncil;
class UAetheriaSliceEnd;
class AAetheriaBridgeDirector;

/**
 *  Drives the command level's screen flow (SLICE_SPEC.md section 6): captain creation, dialogue,
 *  the star map, councils and the slice ending. PC_Command is a Blueprint child.
 */
UCLASS(Blueprintable)
class AAetheriaCommandController : public APlayerController
{
	GENERATED_BODY()

public:

	AAetheriaCommandController();

	// ---- Screens and data ----

	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSoftClassPtr<UAetheriaCaptainCreation> CreationClass;

	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSoftClassPtr<UAetheriaDialogue> DialogueClass;

	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSoftClassPtr<UAetheriaStarMap> StarMapClass;

	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSoftClassPtr<UAetheriaCouncil> CouncilClass;

	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSoftClassPtr<UAetheriaSliceEnd> SliceEndClass;

	UPROPERTY(EditDefaultsOnly, Category="Data")
	TSoftObjectPtr<UDataTable> SequencesTable;

	UPROPERTY(EditDefaultsOnly, Category="Data")
	TSoftObjectPtr<UDataTable> RelayTable;

	UPROPERTY(EditDefaultsOnly, Category="Data")
	TSoftObjectPtr<UDataTable> DirectiveTable;

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TSoftObjectPtr<UWorld> CombatLevel;

	UPROPERTY(EditDefaultsOnly, Category="Flow")
	TSoftObjectPtr<UWorld> CommandLevel;

	// ---- State ----

	UPROPERTY(BlueprintReadOnly, Category="Flow")
	TObjectPtr<UAetheriaGameInstance> GI;

	/** The bridge's camera director, when L_Command has one */
	UPROPERTY(BlueprintReadOnly, Category="Flow")
	TObjectPtr<AAetheriaBridgeDirector> Director;

	/** What happens when the current dialogue ends */
	UPROPERTY(BlueprintReadOnly, Category="Flow")
	FName AfterDialogue;

	// ---- Flow ----

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ShowCreation();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ShowStarMap();

	/** Plays a DT_Sequences row, then continues with AfterKey */
	UFUNCTION(BlueprintCallable, Category="Flow")
	void PlaySequence(FName Row, FName AfterKey);

	/** Plays the given lines, then continues with AfterKey */
	UFUNCTION(BlueprintCallable, Category="Flow")
	void PlayLines(const TArray<FS_Line>& Lines, FName AfterKey);

	/** Called by the star map after a leap lands */
	UFUNCTION(BlueprintCallable, Category="Flow")
	void OnArrive(E_Encounter Encounter);

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ShowCouncil(UDataTable* Table, FName Key);

	/** Applies the Core Architect's disposition after the Directive council */
	UFUNCTION(BlueprintCallable, Category="Flow")
	void ApplyArchitectConsequence();

	UFUNCTION(BlueprintCallable, Category="Flow")
	void ShowSliceEnd();

	/** Resets the campaign and reloads the command level */
	UFUNCTION(BlueprintCallable, Category="Flow")
	void NewCommand();

	// ---- Data-driven acts (Act VII) ----

	/** From the slice's end screen: carry the campaign into Act VII and reload the command level */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void StartActVII();

	/** Carry the campaign into a data-driven act and reload the command level */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void StartAct(int32 Act);

	/** From an end screen: the act after the current one (Act VII after the slice) */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void StartNextAct();

	/** Arrival at a node of a data-driven act: its arrival sequence, then its councils in order */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void RunNode(FName Node);

	/** Holds the next council of the current node whose requirements are met, or shows the star map */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void ContinueNode();

	/** Opens a DT_Councils council */
	UFUNCTION(BlueprintCallable, Category="Act VII")
	void ShowDataCouncil(FName Council);

	/** Called by a data council when the Captain decides */
	void OnCouncilChosen(FName Council, const FS_CouncilOption& Opt);

	/** Focuses Widget for keyboard input, or the viewport when null */
	void FocusWidget(UUserWidget* Widget);

	UDataTable* GetRelayTable() const { return RelayTable.LoadSynchronous(); }
	UDataTable* GetDirectiveTable() const { return DirectiveTable.LoadSynchronous(); }

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSequenceDone();

	UFUNCTION()
	void OnLineShown(E_Speaker Speaker);

	/** Cuts the bridge camera to a named shot, if there is a director */
	void ShowShot(FName Shot);

	/** After a council: the next council or battle its option named */
	FName PendingNextCouncil;
	FName PendingStartBattle;

	/** Whether a council can be held now (requirements, not already held, at least one option available) */
	bool CanHoldCouncil(FName Council) const;

	/** Stages a DT_Battles battle in the combat level */
	void StartBattle(FName Battle);

	/** Back on the bridge after a data-act battle */
	void AfterDataBattle();

	/** After a won battle's debrief: the sequence its BackgroundAftermath names for the Captain's background */
	FName PendingAftermath;

	/** The act's ending: Act VII plays the second revelation by leaning first; then the report and the end screen */
	void PlayActEnd();

	/** The act-end mission report, built from the campaign state, then the end screen */
	void PlayActReport();

	/** Points the view at the level's camera tagged CommandCamera */
	void UseCommandCamera();
};
