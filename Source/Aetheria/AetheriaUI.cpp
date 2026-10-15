// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaUI.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/Layout/Anchors.h"

namespace AetheriaUI
{
	const FLinearColor Cyan(0.2f, 0.85f, 1.0f);
	const FLinearColor Amber(1.0f, 0.7f, 0.15f);
	const FLinearColor Red(1.0f, 0.2f, 0.15f);
	const FLinearColor TextColor(0.88f, 0.92f, 0.96f);
	const FLinearColor Dim(0.55f, 0.65f, 0.75f);
	const FLinearColor PanelBackground(0.01f, 0.02f, 0.05f, 0.88f);
	const FLinearColor ButtonText(0.03f, 0.05f, 0.09f);

	UTextBlock* MakeText(UWidgetTree* Tree, FName Name, const FString& Text, int32 Size, const FLinearColor& Color, bool bWrap)
	{
		UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(FSlateColor(Color));
		Block->SetAutoWrapText(bWrap);

		FSlateFontInfo Font = Block->GetFont();
		Font.Size = Size;
		Block->SetFont(Font);
		return Block;
	}

	UButton* MakeButton(UWidgetTree* Tree, FName Name, const FString& Label, int32 Size, UTextBlock** OutLabel, FName LabelName)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UTextBlock* LabelBlock = MakeText(Tree, LabelName, Label, Size, ButtonText);
		LabelBlock->SetJustification(ETextJustify::Center);
		Button->AddChild(LabelBlock);
		if (OutLabel)
		{
			*OutLabel = LabelBlock;
		}
		return Button;
	}

	UBorder* MakePanel(UWidgetTree* Tree, FName Name, const FLinearColor& Background, const FMargin& Padding)
	{
		UBorder* Panel = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		Panel->SetBrushColor(Background);
		Panel->SetPadding(Padding);
		return Panel;
	}

	UProgressBar* MakeBar(UWidgetTree* Tree, FName Name, const FLinearColor& Fill)
	{
		UProgressBar* Bar = Tree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
		Bar->SetFillColorAndOpacity(Fill);
		Bar->SetPercent(0.0f);
		return Bar;
	}

	void AddToBox(UVerticalBox* Box, UWidget* Child, const FMargin& Padding, bool bFill)
	{
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
		Slot->SetPadding(Padding);
		Slot->SetSize(FSlateChildSize(bFill ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic));
	}

	void AddToRow(UHorizontalBox* Row, UWidget* Child, const FMargin& Padding, bool bFill)
	{
		UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Child);
		Slot->SetPadding(Padding);
		Slot->SetSize(FSlateChildSize(bFill ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic));
	}

	UCanvasPanelSlot* AddToCanvas(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FVector2D& Alignment,
		const FMargin& Offsets, bool bAutoSize)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetOffsets(Offsets);
		Slot->SetAutoSize(bAutoSize);
		return Slot;
	}

	FLinearColor InstabilityColor(float Instability)
	{
		if (Instability >= 80.0f)
		{
			return Red;
		}
		return Instability >= 50.0f ? Amber : Cyan;
	}
}
