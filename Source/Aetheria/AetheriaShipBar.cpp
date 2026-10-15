// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaShipBar.h"
#include "AetheriaShipBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UAetheriaShipBar::SetOwnerShip(AAetheriaShipBase* Ship)
{
	OwnerShip = Ship;

	if (ShieldBar)
	{
		ShieldBar->SetFillColorAndOpacity(ShieldColor);
	}
	if (HullBar && Ship)
	{
		HullBar->SetFillColorAndOpacity(Ship->Faction == E_Faction::Player ? FriendlyHullColor : HostileHullColor);
	}

	Refresh();
}

void UAetheriaShipBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Refresh();
}

void UAetheriaShipBar::Refresh()
{
	const AAetheriaShipBase* Ship = OwnerShip.Get();
	if (!Ship)
	{
		return;
	}

	if (NameText)
	{
		NameText->SetText(Ship->GetBarLabel());
	}
	if (ShieldBar)
	{
		ShieldBar->SetPercent(Ship->MaxShield > 0.0f ? Ship->Shield / Ship->MaxShield : 0.0f);
	}
	if (HullBar)
	{
		HullBar->SetPercent(Ship->MaxHull > 0.0f ? Ship->Hull / Ship->MaxHull : 0.0f);
	}
}
