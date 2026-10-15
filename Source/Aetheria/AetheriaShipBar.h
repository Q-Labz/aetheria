// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AetheriaShipBar.generated.h"

class UTextBlock;
class UProgressBar;
class AAetheriaShipBase;

/**
 *  Floating bar over a ship: name, shield (cyan) and hull (green for the fleet, red for hostiles).
 *  WBP_ShipBar is a Blueprint child that lays out widgets named NameText, ShieldBar and HullBar.
 */
UCLASS(Abstract, Blueprintable)
class UAetheriaShipBar : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Ship Bar")
	void SetOwnerShip(AAetheriaShipBase* Ship);

	UFUNCTION(BlueprintPure, Category="Ship Bar")
	AAetheriaShipBase* GetOwnerShip() const { return OwnerShip.Get(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Bar")
	FLinearColor ShieldColor = FLinearColor(0.0f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Bar")
	FLinearColor FriendlyHullColor = FLinearColor(0.1f, 0.9f, 0.3f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship Bar")
	FLinearColor HostileHullColor = FLinearColor(0.95f, 0.15f, 0.1f);

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Pushes the owner ship's name, shield and hull into the widgets */
	UFUNCTION(BlueprintCallable, Category="Ship Bar")
	void Refresh();

	UPROPERTY(BlueprintReadOnly, Category="Ship Bar", meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(BlueprintReadOnly, Category="Ship Bar", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> ShieldBar;

	UPROPERTY(BlueprintReadOnly, Category="Ship Bar", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> HullBar;

private:

	TWeakObjectPtr<AAetheriaShipBase> OwnerShip;
};
