// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AetheriaTypes.h"
#include "AetheriaCombatHUD.generated.h"

class UTextBlock;
class UProgressBar;
class UScrollBox;
class UWidget;

/**
 *  Tactical combat HUD (SLICE_SPEC.md section 10): fleet status line, pause and beam-aim banners,
 *  flagship bars, beam and Chrono Surge readiness, group cards and the comms log.
 *  WBP_CombatHUD is a Blueprint child that lays out widgets with the names below; any it omits are skipped.
 *  Widgets keep ticking while the game is paused, so the HUD stays live during a tactical pause.
 */
UCLASS(Abstract, Blueprintable)
class UAetheriaCombatHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetPausedBanner(bool bVisible);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetAimBanner(bool bVisible);

	/** Adds "{SpeakerName}: {Text}" to the comms log and scrolls to it */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void Log(E_Speaker Speaker, const FText& Text);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Pulls fleet, beam and campaign state into the widgets */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void Refresh();

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> PauseBanner;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> AimBanner;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> FlagshipHullBar;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> FlagshipShieldBar;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BeamLabel;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> BeamBar;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BeamStatusText;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ChronoLabel;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> ChronoBar;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> GroupsText;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UScrollBox> CommsLog;

	UPROPERTY(BlueprintReadOnly, Category="HUD", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ControlsText;

	/** Font size for comms log lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
	int32 LogFontSize = 11;
};
