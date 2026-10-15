// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AetheriaAfterAction.generated.h"

class UTextBlock;
class UButton;
class AAetheriaCombatManager;

/**
 *  Mission report shown when the battle ends (SLICE_SPEC.md section 10).
 *  Victory offers Return to Command; defeat offers Retry Engagement and Return to Command.
 *  WBP_AfterAction is a Blueprint child laying out TitleText, BodyText, PrimaryButton/PrimaryLabel
 *  and SecondaryButton/SecondaryLabel.
 */
UCLASS(Abstract, Blueprintable)
class UAetheriaAfterAction : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="After Action")
	void Setup(AAetheriaCombatManager* InManager, bool bInVictory, const FText& Title, const FText& Body);

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandlePrimary();

	UFUNCTION()
	void HandleSecondary();

	void ApplyLabels();

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UButton> PrimaryButton;

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> PrimaryLabel;

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UButton> SecondaryButton;

	UPROPERTY(BlueprintReadOnly, Category="After Action", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SecondaryLabel;

	UPROPERTY(BlueprintReadOnly, Category="After Action")
	TObjectPtr<AAetheriaCombatManager> Manager;

	UPROPERTY(BlueprintReadOnly, Category="After Action")
	bool bVictory = false;
};
