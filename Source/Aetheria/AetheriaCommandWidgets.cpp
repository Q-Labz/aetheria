// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCommandWidgets.h"
#include "AetheriaCommandController.h"
#include "AetheriaGameInstance.h"
#include "AetheriaUI.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Rendering/DrawElements.h"
#include "TimerManager.h"
#include "Widgets/Layout/Anchors.h"

using namespace AetheriaUI;

namespace
{
	void SetText(UTextBlock* Block, const FString& Text)
	{
		if (Block)
		{
			Block->SetText(FText::FromString(Text));
		}
	}

	void SetShown(UWidget* Widget, bool bShown, ESlateVisibility Visible = ESlateVisibility::HitTestInvisible)
	{
		if (Widget)
		{
			Widget->SetVisibility(bShown ? Visible : ESlateVisibility::Collapsed);
		}
	}

	void SetEnabled(UWidget* Widget, bool bEnabled)
	{
		if (Widget)
		{
			Widget->SetIsEnabled(bEnabled);
		}
	}

	/** Centred panel on a full-screen canvas; returns the panel's vertical box */
	UVerticalBox* MakeCentredPanel(UWidgetTree* Tree, const FVector2D& Size)
	{
		UCanvasPanel* Root = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		Tree->RootWidget = Root;

		UBorder* Panel = MakePanel(Tree, TEXT("Panel"), PanelBackground, FMargin(36.0f, 30.0f));
		AddToCanvas(Root, Panel, FAnchors(0.5f, 0.5f), FVector2D(0.5f, 0.5f), FMargin(0.0f, 0.0f, Size.X, Size.Y));

		UVerticalBox* Box = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
		Panel->SetContent(Box);
		return Box;
	}

	FString ReplaceTokens(const UAetheriaGameInstance* GI, FString Text)
	{
		if (GI)
		{
			Text.ReplaceInline(TEXT("{Captain}"), *GI->CaptainName);
			Text.ReplaceInline(TEXT("{Background}"), *GI->BackgroundTitle());
		}
		return Text;
	}
}

// ---- UAetheriaCommandWidget --------------------------------------------------------------------

TSharedRef<SWidget> UAetheriaCommandWidget::RebuildWidget()
{
	// only in a running game: thumbnail rendering and editor previews instantiate widgets too,
	// and building (and later constructing) the full screen there can stall the save
	const UWorld* World = GetWorld();
	if (!IsDesignTime() && World && World->IsGameWorld())
	{
		if (!WidgetTree)
		{
			WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
		}
		if (!WidgetTree->RootWidget)
		{
			BuildDefaultLayout();
		}
	}
	return Super::RebuildWidget();
}

void UAetheriaCommandWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
}

UAetheriaGameInstance* UAetheriaCommandWidget::GetAetheriaGameInstance() const
{
	return Cast<UAetheriaGameInstance>(GetGameInstance());
}

AAetheriaCommandController* UAetheriaCommandWidget::GetCommandController() const
{
	return Cast<AAetheriaCommandController>(GetOwningPlayer());
}

// ---- UAetheriaCaptainCreation ------------------------------------------------------------------

void UAetheriaCaptainCreation::BuildDefaultLayout()
{
	UVerticalBox* Box = MakeCentredPanel(WidgetTree, FVector2D(760.0f, 600.0f));

	AddToBox(Box, MakeText(WidgetTree, TEXT("TitleText"), TEXT("CAPTAIN'S COMMISSION"), 22, Cyan), FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddToBox(Box, MakeText(WidgetTree, TEXT("NamePrompt"), TEXT("Captain's name"), 12, Dim));

	NameBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("NameBox"));
	NameBox->SetHintText(FText::FromString(TEXT("Enter the Captain's name")));
	AddToBox(Box, NameBox, FMargin(0.0f, 2.0f, 0.0f, 14.0f));

	AddToBox(Box, MakeText(WidgetTree, TEXT("BackgroundPrompt"), TEXT("Background"), 12, Dim));
	SynodButton = MakeButton(WidgetTree, TEXT("SynodButton"), TEXT("SYNOD ENVOY"));
	GuildButton = MakeButton(WidgetTree, TEXT("GuildButton"), TEXT("GUILD ENGINEER"));
	CoalitionButton = MakeButton(WidgetTree, TEXT("CoalitionButton"), TEXT("COALITION VANGUARD"));
	CorsairButton = MakeButton(WidgetTree, TEXT("CorsairButton"), TEXT("OUTCAST CORSAIR"));
	for (UButton* Button : { SynodButton.Get(), GuildButton.Get(), CoalitionButton.Get(), CorsairButton.Get() })
	{
		AddToBox(Box, Button, FMargin(0.0f, 3.0f));
	}

	BackgroundDesc = MakeText(WidgetTree, TEXT("BackgroundDesc"), TEXT("Choose the Captain's background."), 13, TextColor, true);
	AddToBox(Box, BackgroundDesc, FMargin(0.0f, 14.0f), true);

	ConfirmButton = MakeButton(WidgetTree, TEXT("ConfirmButton"), TEXT("ASSUME COMMAND"), 15);
	AddToBox(Box, ConfirmButton, FMargin(0.0f, 8.0f, 0.0f, 0.0f));
}

void UAetheriaCaptainCreation::NativeConstruct()
{
	Super::NativeConstruct();

	if (NameBox)
	{
		NameBox->OnTextChanged.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleNameChanged);
	}
	if (SynodButton) { SynodButton->OnClicked.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleSynod); }
	if (GuildButton) { GuildButton->OnClicked.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleGuild); }
	if (CoalitionButton) { CoalitionButton->OnClicked.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleCoalition); }
	if (CorsairButton) { CorsairButton->OnClicked.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleCorsair); }
	if (ConfirmButton) { ConfirmButton->OnClicked.AddUniqueDynamic(this, &UAetheriaCaptainCreation::HandleConfirm); }

	UpdateConfirm();
}

FText UAetheriaCaptainCreation::DescribeBackground(E_Background Background)
{
	switch (Background)
	{
	case E_Background::SynodEnvoy:
		return FText::FromString(TEXT("Diplomat of the Synod Key. Fluent in Precursor protocol. Unique dialogue with the Architect."));
	case E_Background::GuildEngineer:
		return FText::FromString(TEXT("Forged in the Guild yards. Stellar Leap costs -10%. Jax Mercer trusts you."));
	case E_Background::CoalitionVanguard:
		return FText::FromString(TEXT("Line officer of the Coalition. Prismatic Beam damage +15%. Tala Vane served with you."));
	case E_Background::OutcastCorsair:
		return FText::FromString(TEXT("Ran with outcast miners. A Corsair gunship joins every battle. Cormaker vouches for you."));
	}
	return FText::GetEmpty();
}

void UAetheriaCaptainCreation::PickBackground(E_Background Background)
{
	PickedBackground = Background;
	bHasPicked = true;

	if (BackgroundDesc)
	{
		BackgroundDesc->SetText(DescribeBackground(Background));
	}

	const TPair<UButton*, E_Background> Buttons[] = {
		{ SynodButton, E_Background::SynodEnvoy }, { GuildButton, E_Background::GuildEngineer },
		{ CoalitionButton, E_Background::CoalitionVanguard }, { CorsairButton, E_Background::OutcastCorsair }
	};
	for (const TPair<UButton*, E_Background>& Entry : Buttons)
	{
		if (Entry.Key)
		{
			Entry.Key->SetBackgroundColor(Entry.Value == Background ? Cyan : FLinearColor::White);
		}
	}

	UpdateConfirm();
}

void UAetheriaCaptainCreation::HandleNameChanged(const FText& Text)
{
	UpdateConfirm();
}

void UAetheriaCaptainCreation::HandleSynod() { PickBackground(E_Background::SynodEnvoy); }
void UAetheriaCaptainCreation::HandleGuild() { PickBackground(E_Background::GuildEngineer); }
void UAetheriaCaptainCreation::HandleCoalition() { PickBackground(E_Background::CoalitionVanguard); }
void UAetheriaCaptainCreation::HandleCorsair() { PickBackground(E_Background::OutcastCorsair); }
void UAetheriaCaptainCreation::HandleConfirm() { Confirm(); }

void UAetheriaCaptainCreation::UpdateConfirm()
{
	const bool bNamed = NameBox && !NameBox->GetText().ToString().TrimStartAndEnd().IsEmpty();
	SetEnabled(ConfirmButton, bNamed && bHasPicked);
}

void UAetheriaCaptainCreation::Confirm()
{
	const FString Name = NameBox ? NameBox->GetText().ToString().TrimStartAndEnd() : FString();
	if (Name.IsEmpty() || !bHasPicked)
	{
		return;
	}

	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->CaptainName = Name;
		GI->Background = PickedBackground;
		GI->ApplyBackgroundStart();
	}

	AAetheriaCommandController* PC = GetCommandController();
	RemoveFromParent();
	if (PC)
	{
		PC->PlaySequence(TEXT("Briefing"), TEXT("ToMap"));
	}
}

// ---- UAetheriaDialogue -------------------------------------------------------------------------

void UAetheriaDialogue::BuildDefaultLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Panel = MakePanel(WidgetTree, TEXT("Panel"), PanelBackground, FMargin(32.0f, 22.0f));
	// stretched across the bottom: left/right are margins, top is position, bottom is height
	AddToCanvas(Root, Panel, FAnchors(0.0f, 1.0f, 1.0f, 1.0f), FVector2D(0.0f, 1.0f), FMargin(120.0f, -40.0f, 120.0f, 230.0f));

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->SetContent(Box);

	SpeakerText = MakeText(WidgetTree, TEXT("SpeakerText"), FString(), 15, Cyan);
	AddToBox(Box, SpeakerText, FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	LineText = MakeText(WidgetTree, TEXT("LineText"), FString(), 17, TextColor, true);
	AddToBox(Box, LineText, FMargin(0.0f), true);

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
	AddToRow(Row, MakeText(WidgetTree, TEXT("Hint"), TEXT("Enter / Space"), 11, Dim), FMargin(0.0f, 6.0f, 0.0f, 0.0f), true);
	ContinueButton = MakeButton(WidgetTree, TEXT("ContinueButton"), TEXT("CONTINUE >"), 13);
	AddToRow(Row, ContinueButton, FMargin(0.0f));
	AddToBox(Box, Row, FMargin(0.0f, 8.0f, 0.0f, 0.0f));
}

void UAetheriaDialogue::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UAetheriaDialogue::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddUniqueDynamic(this, &UAetheriaDialogue::HandleContinue);
	}

	// show the first line now so an early Continue can't skip it; finishing (no visible lines)
	// removes the widget, so that waits for the next tick
	ShowCurrent(true);
}

void UAetheriaDialogue::SetLines(const TArray<FS_Line>& InLines)
{
	Lines = InLines;
	Index = 0;
	bFinished = false;
}

FReply UAetheriaDialogue::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		Continue();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UAetheriaDialogue::HandleContinue()
{
	Continue();
}

void UAetheriaDialogue::Continue()
{
	if (bFinished)
	{
		return;
	}
	++Index;
	ShowCurrent();
}

void UAetheriaDialogue::ShowCurrent(bool bDeferFinish)
{
	if (bFinished)
	{
		return;
	}

	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	while (Lines.IsValidIndex(Index))
	{
		FText Out;
		bool bShow = true;
		if (GI)
		{
			GI->ResolveLine(Lines[Index], Out, bShow);
		}
		else
		{
			Out = Lines[Index].Text;
		}

		if (bShow)
		{
			if (SpeakerText)
			{
				SpeakerText->SetText(GI ? GI->SpeakerName(Lines[Index].Speaker) : FText::GetEmpty());
			}
			if (LineText)
			{
				LineText->SetText(Out);
			}
			OnLineShown.Broadcast(Lines[Index].Speaker);
			return;
		}
		++Index;
	}

	if (bDeferFinish)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() { ShowCurrent(false); }));
			return;
		}
	}

	bFinished = true;
	RemoveFromParent();
	OnDialogueFinished.Broadcast();
}

// ---- UAetheriaNodeButton -----------------------------------------------------------------------

void UAetheriaNodeButton::BuildDefaultLayout()
{
	UTextBlock* Label = nullptr;
	NodeButton = MakeButton(WidgetTree, TEXT("NodeButton"), FString(), 12, &Label, TEXT("NodeLabel"));
	NodeLabel = Label;
	WidgetTree->RootWidget = NodeButton;
}

void UAetheriaNodeButton::NativeConstruct()
{
	Super::NativeConstruct();
	if (NodeButton)
	{
		NodeButton->OnClicked.AddUniqueDynamic(this, &UAetheriaNodeButton::HandleClicked);
	}
	if (NodeLabel && !BaseLabel.IsEmpty())
	{
		NodeLabel->SetText(BaseLabel);
	}
}

void UAetheriaNodeButton::Setup(FName InNodeID, const FText& Label, UAetheriaStarMap* InParentMap)
{
	NodeID = InNodeID;
	ParentMap = InParentMap;
	BaseLabel = Label;
	if (NodeLabel)
	{
		NodeLabel->SetText(Label);
	}
}

void UAetheriaNodeButton::SetState(bool bIsCurrent, bool bIsSelected)
{
	if (NodeLabel)
	{
		const FString Prefix = bIsCurrent ? TEXT("• ") : (bIsSelected ? TEXT("> ") : TEXT(""));
		NodeLabel->SetText(FText::FromString(Prefix + BaseLabel.ToString()));
	}
	if (NodeButton)
	{
		NodeButton->SetBackgroundColor(bIsSelected ? Cyan : (bIsCurrent ? Amber : FLinearColor::White));
	}
}

void UAetheriaNodeButton::HandleClicked()
{
	if (ParentMap)
	{
		ParentMap->SelectNode(NodeID);
	}
}

// ---- UAetheriaStarMap --------------------------------------------------------------------------

void UAetheriaStarMap::BuildDefaultLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// chart on the left 70%
	UBorder* MapPanel = MakePanel(WidgetTree, TEXT("MapPanel"), FLinearColor(0.0f, 0.01f, 0.03f, 0.55f), FMargin(0.0f));
	AddToCanvas(Root, MapPanel, FAnchors(0.0f, 0.0f, 0.7f, 1.0f), FVector2D::ZeroVector, FMargin(0.0f));
	MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MapCanvas"));
	MapPanel->SetContent(MapCanvas);
	AddToCanvas(MapCanvas, MakeText(WidgetTree, TEXT("MapHeader"), TEXT("STELLAR LEAP — SELECT A DESTINATION"), 14, Dim),
		FAnchors(0.0f, 0.0f), FVector2D::ZeroVector, FMargin(28.0f, 22.0f, 0.0f, 0.0f), true);

	// decision panel on the right
	UBorder* InfoPanel = MakePanel(WidgetTree, TEXT("InfoPanel"), PanelBackground, FMargin(26.0f, 24.0f));
	AddToCanvas(Root, InfoPanel, FAnchors(0.7f, 0.0f, 1.0f, 1.0f), FVector2D::ZeroVector, FMargin(0.0f));
	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
	InfoPanel->SetContent(Box);

	NodeTitle = MakeText(WidgetTree, TEXT("NodeTitle"), FString(), 20, Cyan, true);
	AddToBox(Box, NodeTitle);
	NodeDesc = MakeText(WidgetTree, TEXT("NodeDesc"), FString(), 13, TextColor, true);
	AddToBox(Box, NodeDesc, FMargin(0.0f, 4.0f, 0.0f, 16.0f));

	AddToBox(Box, MakeText(WidgetTree, TEXT("InstabilityLabel"), TEXT("STAR-DRIVE INSTABILITY"), 11, Dim));
	USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("InstabilitySize"));
	BarSize->SetHeightOverride(20.0f);
	UOverlay* BarStack = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("InstabilityStack"));
	BarSize->SetContent(BarStack);
	ProjectedBar = MakeBar(WidgetTree, TEXT("ProjectedBar"), FLinearColor(1.0f, 1.0f, 1.0f, 0.35f));
	InstabilityBar = MakeBar(WidgetTree, TEXT("InstabilityBar"), Cyan);
	{
		// the live bar sits over the ghost projection, so its background must be clear
		FProgressBarStyle Style = InstabilityBar->GetWidgetStyle();
		Style.BackgroundImage.TintColor = FSlateColor(FLinearColor::Transparent);
		InstabilityBar->SetWidgetStyle(Style);
	}
	for (UProgressBar* Bar : { ProjectedBar.Get(), InstabilityBar.Get() })
	{
		UOverlaySlot* BarSlot = BarStack->AddChildToOverlay(Bar);
		BarSlot->SetHorizontalAlignment(HAlign_Fill);
		BarSlot->SetVerticalAlignment(VAlign_Fill);
	}
	AddToBox(Box, BarSize, FMargin(0.0f, 4.0f));

	LeapReadout = MakeText(WidgetTree, TEXT("LeapReadout"), FString(), 12, TextColor, true);
	AddToBox(Box, LeapReadout, FMargin(0.0f, 6.0f));
	RuptureWarning = MakeText(WidgetTree, TEXT("RuptureWarning"), FString(), 13, Red, true);
	AddToBox(Box, RuptureWarning, FMargin(0.0f, 4.0f));
	AdvisorBark = MakeText(WidgetTree, TEXT("AdvisorBark"), FString(), 13, Amber, true);
	AddToBox(Box, AdvisorBark, FMargin(0.0f, 10.0f, 0.0f, 16.0f));

	LeapButton = MakeButton(WidgetTree, TEXT("LeapButton"), TEXT("EXECUTE STELLAR LEAP"), 14);
	ChartButton = MakeButton(WidgetTree, TEXT("ChartButton"), TEXT("LYRA: CHART ROUTE"));
	VentButton = MakeButton(WidgetTree, TEXT("VentButton"), TEXT("VENT CHRONO-CORE (−20 / 25 charge)"));
	HoldButton = MakeButton(WidgetTree, TEXT("HoldButton"), TEXT("HOLD STATION (−10 / Pursuit +1)"));
	RecalButton = MakeButton(WidgetTree, TEXT("RecalButton"), TEXT("SPIRE RECALIBRATION (−40)"));
	TreatButton = MakeButton(WidgetTree, TEXT("TreatButton"), TEXT("DR. THORNE: TREAT INJURED"));
	for (UButton* Button : { LeapButton.Get(), ChartButton.Get(), VentButton.Get(), HoldButton.Get(), RecalButton.Get(), TreatButton.Get() })
	{
		AddToBox(Box, Button, FMargin(0.0f, 3.0f));
	}

	UWidget* Spacer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Spacer"));
	AddToBox(Box, Spacer, FMargin(0.0f), true);

	StatusText = MakeText(WidgetTree, TEXT("StatusText"), FString(), 12, Dim, true);
	AddToBox(Box, StatusText, FMargin(0.0f, 8.0f));
	MatrixNote = MakeText(WidgetTree, TEXT("MatrixNote"), TEXT("NEURAL MATRIX: unregistered process present."), 12, Amber, true);
	AddToBox(Box, MatrixNote);

	ToastText = MakeText(WidgetTree, TEXT("ToastText"), FString(), 16, Amber, true);
	ToastText->SetJustification(ETextJustify::Center);
	AddToCanvas(Root, ToastText, FAnchors(0.35f, 0.82f), FVector2D(0.5f, 0.5f), FMargin(0.0f, 0.0f, 900.0f, 90.0f));

	FlashOverlay = MakePanel(WidgetTree, TEXT("FlashOverlay"), FLinearColor::White, FMargin(0.0f));
	AddToCanvas(Root, FlashOverlay, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FVector2D::ZeroVector, FMargin(0.0f));
}

void UAetheriaStarMap::NativeConstruct()
{
	Super::NativeConstruct();

	if (NodeButtonClass.IsNull())
	{
		NodeButtonClass = TSoftClassPtr<UAetheriaNodeButton>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_NodeButton.WBP_NodeButton_C")));
	}

	SetShown(FlashOverlay, false);
	SetShown(ToastText, false);

	UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (!GI)
	{
		return;
	}

	// one button per star, placed at its chart position
	NodeButtons.Reset();
	NodePositions.Reset();
	const UDataTable* Nodes = GI->StarNodesTable.LoadSynchronous();
	UClass* ButtonClass = NodeButtonClass.LoadSynchronous();
	if (Nodes && ButtonClass && MapCanvas)
	{
		Nodes->ForeachRow<FS_StarNode>(TEXT("StarMap"), [this, ButtonClass](const FName& RowName, const FS_StarNode& Node)
		{
			UAetheriaNodeButton* Button = CreateWidget<UAetheriaNodeButton>(GetOwningPlayer(), ButtonClass);
			if (!Button)
			{
				return;
			}
			Button->Setup(Node.NodeID, Node.DisplayName, this);
			UCanvasPanelSlot* NodeSlot = MapCanvas->AddChildToCanvas(Button);
			NodeSlot->SetPosition(Node.MapPos);
			NodeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			NodeSlot->SetAutoSize(true);
			NodeButtons.Add(Node.NodeID, Button);
			NodePositions.Add(Node.NodeID, Node.MapPos);
		});
	}

	if (LeapButton) { LeapButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleLeap); }
	if (ChartButton) { ChartButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleChart); }
	if (VentButton) { VentButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleVent); }
	if (HoldButton) { HoldButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleHold); }
	if (RecalButton) { RecalButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleRecalibrate); }
	if (TreatButton) { TreatButton->OnClicked.AddUniqueDynamic(this, &UAetheriaStarMap::HandleTreat); }

	GI->OnCampaignStateChanged.AddUniqueDynamic(this, &UAetheriaStarMap::HandleStateChanged);
	Refresh();
}

void UAetheriaStarMap::NativeDestruct()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->OnCampaignStateChanged.RemoveDynamic(this, &UAetheriaStarMap::HandleStateChanged);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	Super::NativeDestruct();
}

void UAetheriaStarMap::HandleStateChanged()
{
	Refresh();
}

void UAetheriaStarMap::SelectNode(FName NodeID)
{
	if (!bBusy)
	{
		SelectedNode = NodeID;
		Refresh();
	}
}

void UAetheriaStarMap::Refresh()
{
	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (!GI)
	{
		return;
	}

	for (const TPair<FName, TObjectPtr<UAetheriaNodeButton>>& Entry : NodeButtons)
	{
		if (Entry.Value)
		{
			Entry.Value->SetState(Entry.Key == GI->CurrentNode, Entry.Key == SelectedNode);
		}
	}

	const UDataTable* Nodes = GI->StarNodesTable.LoadSynchronous();
	const bool bHasDestination = !SelectedNode.IsNone() && SelectedNode != GI->CurrentNode;
	const FName Shown = bHasDestination ? SelectedNode : GI->CurrentNode;
	if (const FS_StarNode* Node = Nodes ? Nodes->FindRow<FS_StarNode>(Shown, TEXT("StarMap"), false) : nullptr)
	{
		SetText(NodeTitle, bHasDestination ? Node->DisplayName.ToString()
			: FString::Printf(TEXT("%s — FLEET POSITION"), *Node->DisplayName.ToString()));
		if (NodeDesc)
		{
			NodeDesc->SetText(Node->Description);
		}
	}

	float Cost = 0.0f, Projected = GI->Instability;
	bool bWillOverflow = false, bFound = false;
	bool bCharted = false;
	if (bHasDestination)
	{
		GI->GetLeapCost(SelectedNode, Cost, Projected, bWillOverflow, bFound);
		bCharted = GI->ChartedRoutes.Contains(GI->RouteName(SelectedNode));
	}

	// readout and warning
	if (!bHasDestination)
	{
		SetText(LeapReadout, TEXT("Select a destination on the chart."));
	}
	else if (!bFound)
	{
		SetText(LeapReadout, TEXT("No direct lane."));
	}
	else
	{
		const UDataTable* Routes = GI->LeapRoutesTable.LoadSynchronous();
		const FS_LeapRoute* Route = Routes ? Routes->FindRow<FS_LeapRoute>(GI->RouteName(SelectedNode), TEXT("StarMap"), false) : nullptr;

		TArray<FString> Parts;
		Parts.Add(FString::Printf(TEXT("Base %g"), Route ? Route->BaseCost : Cost));
		if (bCharted) { Parts.Add(TEXT("Charted ×0.5")); }
		if (GI->Instability >= 50.0f) { Parts.Add(TEXT("Strain ×1.25")); }
		if (GI->Background == E_Background::GuildEngineer) { Parts.Add(TEXT("Guild ×0.9")); }
		Parts.Add(FString::Printf(TEXT("Projected %g / 100"), FMath::RoundToFloat(Projected * 100.0f) / 100.0f));
		SetText(LeapReadout, FString::Join(Parts, TEXT(" | ")));
	}

	if (bFound && bWillOverflow)
	{
		float HullLoss;
		int32 Injured, Desync;
		GI->PreviewRupture(Projected - 100.0f, HullLoss, Injured, Desync);
		SetText(RuptureWarning, FString::Printf(TEXT("PROJECTED OVERFLOW. Expected rupture: hull −%d%%, %d crew injured, %d Phantom vessels desynchronised."),
			FMath::RoundToInt(HullLoss * 100.0f), Injured, Desync));
		SetShown(RuptureWarning, true);
	}
	else
	{
		SetShown(RuptureWarning, false);
	}

	// crew advise; the Captain decides
	FString Bark;
	if (!bHasDestination || !bFound)
	{
		Bark = TEXT("VEX AI: Awaiting your order, Captain.");
	}
	else if (bWillOverflow)
	{
		Bark = TEXT("VEX AI: Captain, this leap exceeds containment tolerance. I advise against it. The decision is yours.");
	}
	else if (!bCharted)
	{
		Bark = TEXT("LYRA: Give me one watch, Captain, and I can weave this lane. It will cost us time.");
	}
	else if (GI->PursuitClock >= 2)
	{
		Bark = TEXT("TALA VANE: Every hour we hold, Vance gains on us. I recommend we move.");
	}
	else
	{
		Bark = TEXT("VEX AI: Lane is within tolerance. Awaiting your order.");
	}
	SetText(AdvisorBark, Bark);

	// bars
	if (InstabilityBar)
	{
		InstabilityBar->SetPercent(GI->Instability / 100.0f);
		InstabilityBar->SetFillColorAndOpacity(InstabilityColor(GI->Instability));
	}
	if (ProjectedBar)
	{
		ProjectedBar->SetPercent(FMath::Min(Projected, 100.0f) / 100.0f);
		FLinearColor Ghost = InstabilityColor(Projected);
		Ghost.A = 0.35f;
		ProjectedBar->SetFillColorAndOpacity(Ghost);
	}

	// actions
	SetEnabled(LeapButton, !bBusy && bFound);
	SetEnabled(ChartButton, !bBusy && bFound && !bCharted);
	SetEnabled(VentButton, !bBusy && GI->ChronoCharge >= 25.0f);
	SetEnabled(HoldButton, !bBusy);
	SetEnabled(RecalButton, !bBusy && GI->SpireRecalibrations > 0);
	SetEnabled(TreatButton, !bBusy && GI->BioGelDoses > 0 && GI->CrewInjured > 0);

	SetText(StatusText, FString::Printf(TEXT("CHRONO %d/100 · HULL %d%% · INJURED %d · BIO-GEL %d · PHANTOM FLEET %d/200 · PURSUIT %d"),
		FMath::RoundToInt(GI->ChronoCharge), FMath::RoundToInt(GI->FlagshipHullPct * 100.0f), GI->CrewInjured,
		GI->BioGelDoses, GI->PhantomShipsActive, GI->PursuitClock));
	SetShown(MatrixNote, GI->ArchitectExposure > 0);
}

int32 UAetheriaStarMap::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	const UDataTable* Routes = GI ? GI->LeapRoutesTable.Get() : nullptr;
	if (!Routes || !MapCanvas)
	{
		return MaxLayer;
	}

	// lanes from map space into this widget's space
	const FGeometry& CanvasGeometry = MapCanvas->GetCachedGeometry();
	auto ToLocal = [&](const FVector2D& MapPos)
	{
		return AllottedGeometry.AbsoluteToLocal(CanvasGeometry.LocalToAbsolute(MapPos));
	};

	TSet<FString> Drawn;
	Routes->ForeachRow<FS_LeapRoute>(TEXT("StarMapPaint"), [&](const FName& RowName, const FS_LeapRoute& Route)
	{
		const FVector2D* From = NodePositions.Find(Route.FromNode);
		const FVector2D* To = NodePositions.Find(Route.ToNode);
		const FString Key = Route.FromNode.LexicalLess(Route.ToNode)
			? Route.FromNode.ToString() + TEXT("|") + Route.ToNode.ToString()
			: Route.ToNode.ToString() + TEXT("|") + Route.FromNode.ToString();
		if (!From || !To || Drawn.Contains(Key))
		{
			return;
		}
		Drawn.Add(Key);

		const bool bCharted = GI->ChartedRoutes.Contains(FName(*(Route.FromNode.ToString() + TEXT("_") + Route.ToNode.ToString())))
			|| GI->ChartedRoutes.Contains(FName(*(Route.ToNode.ToString() + TEXT("_") + Route.FromNode.ToString())));

		TArray<FVector2D> Points = { ToLocal(*From), ToLocal(*To) };
		FSlateDrawElement::MakeLines(OutDrawElements, MaxLayer + 1, AllottedGeometry.ToPaintGeometry(), Points,
			ESlateDrawEffect::None, bCharted ? Cyan : FLinearColor(0.4f, 0.45f, 0.5f, 0.8f), true, bCharted ? 3.0f : 2.0f);
	});

	return MaxLayer + 1;
}

void UAetheriaStarMap::SetBusy(bool bInBusy)
{
	bBusy = bInBusy;
	Refresh();
}

void UAetheriaStarMap::ShowToast(const FText& Text, float Seconds)
{
	if (!ToastText)
	{
		return;
	}
	ToastText->SetText(Text);
	SetShown(ToastText, true);
	GetWorld()->GetTimerManager().SetTimer(ToastTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		SetShown(ToastText, false);
	}), Seconds, false);
}

void UAetheriaStarMap::HandleLeap()
{
	UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (bBusy || !GI || SelectedNode.IsNone())
	{
		return;
	}

	SetBusy(true);
	SetShown(FlashOverlay, true);
	FTimerManager& Timers = GetWorld()->GetTimerManager();
	Timers.SetTimer(FlashTimer, FTimerDelegate::CreateWeakLambda(this, [this]() { SetShown(FlashOverlay, false); }), 0.6f, false);

	const E_Encounter Encounter = GI->ExecuteLeap(SelectedNode);
	SelectedNode = NAME_None;

	Timers.SetTimer(ToastTimer, FTimerDelegate::CreateWeakLambda(this, [this, GI]()
	{
		ShowToast(GI->LastReport, 2.5f);
	}), 0.6f, false);

	Timers.SetTimer(ArriveTimer, FTimerDelegate::CreateWeakLambda(this, [this, Encounter]()
	{
		bBusy = false;
		if (AAetheriaCommandController* PC = GetCommandController())
		{
			PC->OnArrive(Encounter);
		}
	}), 2.1f, false);
}

void UAetheriaStarMap::HandleChart()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->ChartRoute(SelectedNode);
		Refresh();
	}
}

void UAetheriaStarMap::HandleVent()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->VentChrono();
		Refresh();
	}
}

void UAetheriaStarMap::HandleHold()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->HoldStation();
		Refresh();
	}
}

void UAetheriaStarMap::HandleRecalibrate()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->SpireRecalibrate();
		Refresh();
		ShowToast(GI->LastReport, 4.0f);
	}
}

void UAetheriaStarMap::HandleTreat()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->TreatInjured();
		Refresh();
	}
}

// ---- UAetheriaOptionButton ---------------------------------------------------------------------

void UAetheriaOptionButton::BuildDefaultLayout()
{
	OptionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OptionButton"));
	WidgetTree->RootWidget = OptionButton;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionContent"));
	if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(OptionButton->AddChild(Box)))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	LabelText = MakeText(WidgetTree, TEXT("LabelText"), FString(), 15, ButtonText, true);
	HintText = MakeText(WidgetTree, TEXT("HintText"), FString(), 12, FLinearColor(0.2f, 0.25f, 0.35f), true);
	AddToBox(Box, LabelText, FMargin(6.0f, 4.0f, 6.0f, 0.0f));
	AddToBox(Box, HintText, FMargin(6.0f, 2.0f, 6.0f, 4.0f));
}

void UAetheriaOptionButton::NativeConstruct()
{
	Super::NativeConstruct();
	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UAetheriaOptionButton::HandleClicked);
	}
	// the Architect's leaning is never shown, only the label and who approves
	if (LabelText)
	{
		LabelText->SetText(Option.Label);
	}
	if (HintText)
	{
		HintText->SetText(Option.ApprovalHint);
	}
}

void UAetheriaOptionButton::Setup(const FS_CouncilOption& InOption, UAetheriaCouncil* InParentCouncil)
{
	Option = InOption;
	ParentCouncil = InParentCouncil;
	if (LabelText)
	{
		LabelText->SetText(Option.Label);
	}
	if (HintText)
	{
		HintText->SetText(Option.ApprovalHint);
	}
}

void UAetheriaOptionButton::HandleClicked()
{
	if (ParentCouncil)
	{
		ParentCouncil->ChooseOption(Option);
	}
}

// ---- UAetheriaCouncil --------------------------------------------------------------------------

void UAetheriaCouncil::GetCouncilText(FName Key, E_Speaker& OutSituationSpeaker, FString& OutSituation, TArray<FS_Counsel>& OutCounsel)
{
	auto Counsel = [&OutCounsel](E_Speaker Officer, const TCHAR* Advice)
	{
		FS_Counsel Entry;
		Entry.Officer = Officer;
		Entry.Advice = FText::FromString(Advice);
		OutCounsel.Add(Entry);
	};

	OutCounsel.Reset();
	if (Key == TEXT("Relay"))
	{
		OutSituationSpeaker = E_Speaker::VexAI;
		OutSituation = TEXT("The Ember Relay has awakened. It transmits on the Zero Beacon carrier and carries the authentication glyph of the Omega Arch. The Core Architect requests a direct link to the Neural Command Matrix to break a Severance-era cipher on the Beacon's second layer. I must note that the Architect never disclosed it could reach us here.");
		Counsel(E_Speaker::TalaVane, TEXT("We do not hand the keys to two hundred warships to a voice we cannot see. Refuse it, or cage it."));
		Counsel(E_Speaker::NixNull, TEXT("I can build it a sandbox inside the Matrix. The isolation field burns Chrono-Core charge, but it stays on our terms."));
		Counsel(E_Speaker::Lyra, TEXT("The Weave around this relay is old and patient. It isn't hostile. It's waiting to learn what we are."));
		Counsel(E_Speaker::JaxMercer, TEXT("Whatever you choose, the relay is drawing power off our hull. Choose soon."));
		Counsel(E_Speaker::ArisThorne, TEXT("Matrix links pass through the crew's neural lattices. If this goes wrong, I'll be the one treating it."));
		Counsel(E_Speaker::VexAI, TEXT("The options are before you. The decision is yours, Captain."));
	}
	else
	{
		OutSituationSpeaker = E_Speaker::CoreArchitect;
		OutSituation = TEXT("I was built with two directives, Key-Bearer, and they no longer agree. Preserve the Arch. Preserve those who pass through it. One of them must govern me. I will take my answer from you.");
		Counsel(E_Speaker::TalaVane, TEXT("Don't let it make this our responsibility, Captain... though I suppose it already has."));
		Counsel(E_Speaker::Lyra, TEXT("Whatever you tell it, it will become."));
		Counsel(E_Speaker::VexAI, TEXT("I offer no recommendation. This is a matter of command."));
	}
}

void UAetheriaCouncil::BuildDefaultLayout()
{
	UVerticalBox* Box = MakeCentredPanel(WidgetTree, FVector2D(1180.0f, 860.0f));

	TitleText = MakeText(WidgetTree, TEXT("TitleText"), TEXT("COUNCIL OF COMMAND"), 22, Cyan);
	AddToBox(Box, TitleText, FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	SituationText = MakeText(WidgetTree, TEXT("SituationText"), FString(), 14, TextColor, true);
	AddToBox(Box, SituationText, FMargin(0.0f, 0.0f, 0.0f, 14.0f));

	AddToBox(Box, MakeText(WidgetTree, TEXT("CounselLabel"), TEXT("COUNSEL"), 11, Dim));
	CounselBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CounselBox"));
	AddToBox(Box, CounselBox, FMargin(0.0f, 2.0f, 0.0f, 14.0f));

	AddToBox(Box, MakeText(WidgetTree, TEXT("OrdersLabel"), TEXT("YOUR ORDERS, CAPTAIN"), 11, Dim));
	OptionBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionBox"));
	AddToBox(Box, OptionBox, FMargin(0.0f, 2.0f), true);
}

void UAetheriaCouncil::Setup(UDataTable* InCouncilTable, FName InCouncilKey)
{
	CouncilTable = InCouncilTable;
	CouncilKey = InCouncilKey;
	bChosen = false;
	if (GetCachedWidget().IsValid())
	{
		Populate();
	}
}

void UAetheriaCouncil::NativeConstruct()
{
	Super::NativeConstruct();

	if (OptionButtonClass.IsNull())
	{
		OptionButtonClass = TSoftClassPtr<UAetheriaOptionButton>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_OptionButton.WBP_OptionButton_C")));
	}
	Populate();
}

void UAetheriaCouncil::Populate()
{
	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();

	E_Speaker SituationSpeaker;
	FString Situation;
	TArray<FS_Counsel> Counsel;
	GetCouncilText(CouncilKey, SituationSpeaker, Situation, Counsel);

	if (SituationText)
	{
		const FString Speaker = GI ? GI->SpeakerName(SituationSpeaker).ToString() : FString();
		SituationText->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Speaker, *ReplaceTokens(GI, Situation))));
	}

	if (CounselBox)
	{
		CounselBox->ClearChildren();
		for (const FS_Counsel& Entry : Counsel)
		{
			const FText* Override = GI ? Entry.BackgroundOverride.Find(GI->Background) : nullptr;
			const FString Speaker = GI ? GI->SpeakerName(Entry.Officer).ToString() : FString();
			const FString Advice = ReplaceTokens(GI, (Override ? *Override : Entry.Advice).ToString());
			AddToBox(CounselBox, MakeText(WidgetTree, NAME_None, FString::Printf(TEXT("%s: %s"), *Speaker, *Advice), 13, TextColor, true));
		}
	}

	if (OptionBox && CouncilTable)
	{
		OptionBox->ClearChildren();
		UClass* ButtonClass = OptionButtonClass.LoadSynchronous();
		CouncilTable->ForeachRow<FS_CouncilOption>(TEXT("Council"), [this, GI, ButtonClass](const FName& RowName, const FS_CouncilOption& Option)
		{
			if (Option.RequiresBackground && (!GI || Option.RequiredBackground != GI->Background))
			{
				return;
			}
			if (UAetheriaOptionButton* Button = ButtonClass ? CreateWidget<UAetheriaOptionButton>(GetOwningPlayer(), ButtonClass) : nullptr)
			{
				Button->Setup(Option, this);
				AddToBox(OptionBox, Button, FMargin(0.0f, 4.0f));
			}
		});
	}
}

void UAetheriaCouncil::ChooseOption(const FS_CouncilOption& Opt)
{
	if (bChosen)
	{
		return;
	}
	bChosen = true;

	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->ApplyOption(Opt);
	}

	AAetheriaCommandController* PC = GetCommandController();
	const FName After = CouncilKey == TEXT("Relay") ? FName(TEXT("AfterRelay")) : FName(TEXT("AfterDirective"));
	RemoveFromParent();
	if (PC)
	{
		PC->PlayLines(Opt.Outcome, After);
	}
}

// ---- UAetheriaSliceEnd -------------------------------------------------------------------------

void UAetheriaSliceEnd::BuildDefaultLayout()
{
	UVerticalBox* Box = MakeCentredPanel(WidgetTree, FVector2D(940.0f, 700.0f));

	TitleText = MakeText(WidgetTree, TEXT("TitleText"), TEXT("END OF VERTICAL SLICE — ACT VI: THE FAR SHORE"), 22, Cyan, true);
	AddToBox(Box, TitleText, FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	SummaryText = MakeText(WidgetTree, TEXT("SummaryText"), FString(), 15, TextColor, true);
	AddToBox(Box, SummaryText, FMargin(0.0f), true);

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	NewCommandButton = MakeButton(WidgetTree, TEXT("NewCommandButton"), TEXT("NEW COMMAND"), 15);
	QuitButton = MakeButton(WidgetTree, TEXT("QuitButton"), TEXT("QUIT"), 15);
	AddToRow(Row, NewCommandButton, FMargin(0.0f, 0.0f, 16.0f, 0.0f));
	AddToRow(Row, QuitButton, FMargin(0.0f));
	AddToBox(Box, Row, FMargin(0.0f, 16.0f, 0.0f, 0.0f));
}

void UAetheriaSliceEnd::NativeConstruct()
{
	Super::NativeConstruct();

	if (NewCommandButton) { NewCommandButton->OnClicked.AddUniqueDynamic(this, &UAetheriaSliceEnd::HandleNewCommand); }
	if (QuitButton) { QuitButton->OnClicked.AddUniqueDynamic(this, &UAetheriaSliceEnd::HandleQuit); }

	const UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (!GI || !SummaryText)
	{
		return;
	}

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Captain %s — %s"), *GI->CaptainName, *GI->BackgroundTitle()));
	Lines.Add(FString::Printf(TEXT("Star-Drive instability: %d / 100"), FMath::RoundToInt(GI->Instability)));
	Lines.Add(FString::Printf(TEXT("Crystal Dreadnought hull integrity: %d%%"), FMath::RoundToInt(GI->FlagshipHullPct * 100.0f)));
	Lines.Add(FString::Printf(TEXT("Crew injured: %d"), GI->CrewInjured));
	Lines.Add(FString::Printf(TEXT("Phantom Fleet: %d of 200"), GI->PhantomShipsActive));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("CREW LOYALTY"));
	for (const TPair<E_Speaker, int32>& Loyalty : GI->CrewLoyalty)
	{
		Lines.Add(FString::Printf(TEXT("  %s: %d"), *GI->SpeakerName(Loyalty.Key).ToString(), Loyalty.Value));
	}
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("ARCHITECT DISPOSITION: %s (%d)"), *GI->LeaningState().ToString(), GI->ArchitectLeaning));

	SummaryText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UAetheriaSliceEnd::HandleNewCommand()
{
	if (AAetheriaCommandController* PC = GetCommandController())
	{
		PC->NewCommand();
	}
}

void UAetheriaSliceEnd::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
