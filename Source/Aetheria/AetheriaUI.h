// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"

class UWidgetTree;
class UWidget;
class UTextBlock;
class UButton;
class UBorder;
class UProgressBar;
class UVerticalBox;
class UHorizontalBox;
class UCanvasPanel;
class UCanvasPanelSlot;
struct FAnchors;

/**
 *  Helpers for the command-layer widgets, which build a default layout in C++ when their
 *  Widget Blueprint child has none. Widgets are named so BindWidgetOptional picks them up either way.
 */
namespace AetheriaUI
{
	/** Palette shared by the command screens */
	extern const FLinearColor Cyan;
	extern const FLinearColor Amber;
	extern const FLinearColor Red;
	extern const FLinearColor TextColor;
	extern const FLinearColor Dim;
	extern const FLinearColor PanelBackground;
	extern const FLinearColor ButtonText;

	UTextBlock* MakeText(UWidgetTree* Tree, FName Name, const FString& Text, int32 Size, const FLinearColor& Color, bool bWrap = false);

	/** A button holding a text label; OutLabel receives the label if wanted */
	UButton* MakeButton(UWidgetTree* Tree, FName Name, const FString& Label, int32 Size = 13, UTextBlock** OutLabel = nullptr, FName LabelName = NAME_None);

	UBorder* MakePanel(UWidgetTree* Tree, FName Name, const FLinearColor& Background, const FMargin& Padding);

	UProgressBar* MakeBar(UWidgetTree* Tree, FName Name, const FLinearColor& Fill);

	/** Appends Child to Box; bFill gives it the remaining height */
	void AddToBox(UVerticalBox* Box, UWidget* Child, const FMargin& Padding = FMargin(0.0f, 2.0f), bool bFill = false);

	void AddToRow(UHorizontalBox* Row, UWidget* Child, const FMargin& Padding = FMargin(0.0f, 0.0f, 8.0f, 0.0f), bool bFill = false);

	/** Adds Child to Canvas with the given anchors, alignment and offsets (position/size, or margins when stretched) */
	UCanvasPanelSlot* AddToCanvas(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FVector2D& Alignment,
		const FMargin& Offsets, bool bAutoSize = false);

	/** Instability colour: cyan below 50, amber below 80, red at or above 80 */
	FLinearColor InstabilityColor(float Instability);
}
