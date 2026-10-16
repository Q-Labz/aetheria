// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AetheriaTypes.h"
#include "AetheriaCommandWidgets.generated.h"

class UTextBlock;
class UButton;
class UBorder;
class UImage;
class UProgressBar;
class UVerticalBox;
class UCanvasPanel;
class UEditableTextBox;
class UDataTable;
class UAetheriaGameInstance;
class AAetheriaCommandController;
class UAetheriaNodeButton;
class UAetheriaOptionButton;

/**
 *  Base for the command-layer screens (SLICE_SPEC.md section 6). If the Widget Blueprint child has no
 *  designer layout, BuildDefaultLayout() builds one in C++; either way the named widgets bind.
 */
UCLASS(Abstract)
class UAetheriaCommandWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Screens take keyboard focus (UI-only input mode) */
	virtual void NativeOnInitialized() override;

	/** Builds the widget tree when the Blueprint provides none */
	virtual void BuildDefaultLayout() {}

	UAetheriaGameInstance* GetAetheriaGameInstance() const;
	AAetheriaCommandController* GetCommandController() const;
};

// ---------------------------------------------------------------------------------------------

/** Name and background selection. WBP_CaptainCreation is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaCaptainCreation : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Captain Creation")
	void PickBackground(E_Background Background);

	UFUNCTION(BlueprintCallable, Category="Captain Creation")
	void Confirm();

	UFUNCTION(BlueprintPure, Category="Captain Creation")
	static FText DescribeBackground(E_Background Background);

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleNameChanged(const FText& Text);

	UFUNCTION() void HandleSynod();
	UFUNCTION() void HandleGuild();
	UFUNCTION() void HandleCoalition();
	UFUNCTION() void HandleCorsair();
	UFUNCTION() void HandleConfirm();

	void UpdateConfirm();

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UEditableTextBox> NameBox;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UButton> SynodButton;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UButton> GuildButton;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UButton> CoalitionButton;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UButton> CorsairButton;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BackgroundDesc;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation", meta=(BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation")
	E_Background PickedBackground = E_Background::SynodEnvoy;

	UPROPERTY(BlueprintReadOnly, Category="Captain Creation")
	bool bHasPicked = false;
};

// ---------------------------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLine, E_Speaker, Speaker);

/** Plays a list of lines, skipping those whose RequiredFlag is not set. WBP_Dialogue is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaDialogue : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SetLines(const TArray<FS_Line>& InLines);

	/** Next line; past the last one raises OnDialogueFinished and removes the widget */
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void Continue();

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnDialogueFinished OnDialogueFinished;

	/** Raised as each visible line is shown, with its speaker (the bridge director frames them) */
	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnDialogueLine OnLineShown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(ExposeOnSpawn="true"))
	TArray<FS_Line> Lines;

	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	int32 Index = 0;

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void HandleContinue();

	/** Shows the next visible line; bDeferFinish ends the dialogue on the next tick instead of now */
	void ShowCurrent(bool bDeferFinish = false);

	UPROPERTY(BlueprintReadOnly, Category="Dialogue", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(BlueprintReadOnly, Category="Dialogue", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LineText;

	UPROPERTY(BlueprintReadOnly, Category="Dialogue", meta=(BindWidgetOptional))
	TObjectPtr<UButton> ContinueButton;

	bool bFinished = false;
};

// ---------------------------------------------------------------------------------------------

class UAetheriaStarMap;

/** A star on the map. WBP_NodeButton is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaNodeButton : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Star Map")
	void Setup(FName InNodeID, const FText& Label, UAetheriaStarMap* InParentMap);

	/** Marks the node as the fleet's position and/or the selected destination */
	void SetState(bool bIsCurrent, bool bIsSelected);

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(ExposeOnSpawn="true"))
	FName NodeID;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(ExposeOnSpawn="true"))
	TObjectPtr<UAetheriaStarMap> ParentMap;

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClicked();

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> NodeButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeLabel;

	FText BaseLabel;
};

// ---------------------------------------------------------------------------------------------

/** The Stellar Leap decision screen. WBP_StarMap is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaStarMap : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Star Map")
	void SelectNode(FName NodeID);

	UFUNCTION(BlueprintCallable, Category="Star Map")
	void Refresh();

	UPROPERTY(EditDefaultsOnly, Category="Star Map")
	TSoftClassPtr<UAetheriaNodeButton> NodeButtonClass;

	UPROPERTY(BlueprintReadOnly, Category="Star Map")
	FName SelectedNode;

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION() void HandleStateChanged();
	UFUNCTION() void HandleLeap();
	UFUNCTION() void HandleChart();
	UFUNCTION() void HandleVent();
	UFUNCTION() void HandleHold();
	UFUNCTION() void HandleRecalibrate();
	UFUNCTION() void HandleTreat();
	UFUNCTION() void HandleOrderBioGel();
	UFUNCTION() void HandleOrderSeeds();
	UFUNCTION() void HandleOrderRefit();
	UFUNCTION() void HandleOrderSpire();
	void Order(FName Kind);

	void ShowToast(const FText& Text, float Seconds);
	void SetBusy(bool bInBusy);

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> MapCanvas;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeTitle;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NodeDesc;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> ProjectedBar;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> InstabilityBar;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LeapReadout;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> RuptureWarning;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> AdvisorBark;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> LeapButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> ChartButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> VentButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> HoldButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> RecalButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> TreatButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	/** Act VII: Outpost One supply orders */
	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SupplyLabel;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UWidget> SupplyRow;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> OrderBioGelButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> OrderSeedsButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> OrderRefitButton;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UButton> OrderSpireButton;

	/** Act VII: standing, the cure and the supply line */
	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ActStatusText;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> MatrixNote;

	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ToastText;

	/** Full-screen white flash played on a leap */
	UPROPERTY(BlueprintReadOnly, Category="Star Map", meta=(BindWidgetOptional))
	TObjectPtr<UBorder> FlashOverlay;

	UPROPERTY()
	TMap<FName, TObjectPtr<UAetheriaNodeButton>> NodeButtons;

	/** Node positions on MapCanvas, for drawing lanes */
	TMap<FName, FVector2D> NodePositions;

	bool bBusy = false;
	FTimerHandle FlashTimer;
	FTimerHandle ToastTimer;
	FTimerHandle ArriveTimer;
};

// ---------------------------------------------------------------------------------------------

class UAetheriaCouncil;

/** One council choice: label and approval hint, never the leaning. WBP_OptionButton is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaOptionButton : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Council")
	void Setup(const FS_CouncilOption& InOption, UAetheriaCouncil* InParentCouncil);

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(ExposeOnSpawn="true"))
	FS_CouncilOption Option;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(ExposeOnSpawn="true"))
	TObjectPtr<UAetheriaCouncil> ParentCouncil;

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClicked();

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> HintText;
};

// ---------------------------------------------------------------------------------------------

/** Council of Command: situation, counsel, and the options this Captain may choose. WBP_Council is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaCouncil : public UAetheriaCommandWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Council")
	void Setup(UDataTable* InCouncilTable, FName InCouncilKey);

	/** A council from DT_Councils (Act VII): situation, counsel and options all come from the row */
	UFUNCTION(BlueprintCallable, Category="Council")
	void SetupDef(FName InCouncilDef);

	/** DT_Councils row, or None for the slice's Relay / Directive councils */
	UPROPERTY(BlueprintReadOnly, Category="Council")
	FName CouncilDef = NAME_None;

	UFUNCTION(BlueprintCallable, Category="Council")
	void ChooseOption(const FS_CouncilOption& Opt);

	UPROPERTY(EditDefaultsOnly, Category="Council")
	TSoftClassPtr<UAetheriaOptionButton> OptionButtonClass;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(ExposeOnSpawn="true"))
	TObjectPtr<UDataTable> CouncilTable;

	/** "Relay" or "Directive" */
	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(ExposeOnSpawn="true"))
	FName CouncilKey;

	/** Situation line and counsel for a council (SLICE_SPEC.md section 11) */
	static void GetCouncilText(FName Key, E_Speaker& OutSituationSpeaker, FString& OutSituation, TArray<FS_Counsel>& OutCounsel);

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	void Populate();

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SituationText;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> CounselBox;

	UPROPERTY(BlueprintReadOnly, Category="Council", meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> OptionBox;

	bool bChosen = false;
};

// ---------------------------------------------------------------------------------------------

/** End-of-slice summary with New Command and Quit. WBP_SliceEnd is a Blueprint child. */
UCLASS(Abstract, Blueprintable)
class UAetheriaSliceEnd : public UAetheriaCommandWidget
{
	GENERATED_BODY()

protected:

	virtual void BuildDefaultLayout() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleNewCommand();

	UFUNCTION()
	void HandleQuit();

	UFUNCTION()
	void HandleContinue();

	UPROPERTY(BlueprintReadOnly, Category="Slice End", meta=(BindWidgetOptional))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(BlueprintReadOnly, Category="Slice End", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, Category="Slice End", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(BlueprintReadOnly, Category="Slice End", meta=(BindWidgetOptional))
	TObjectPtr<UButton> NewCommandButton;

	UPROPERTY(BlueprintReadOnly, Category="Slice End", meta=(BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;
};
