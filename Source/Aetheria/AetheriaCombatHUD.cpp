// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCombatHUD.h"
#include "AetheriaCombatManager.h"
#include "AetheriaFlagship.h"
#include "AetheriaGameInstance.h"
#include "AetheriaPhantomSquadron.h"
#include "AetheriaTacticalController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

namespace AetheriaHUD
{
	void ShowWidget(UWidget* Widget, bool bVisible)
	{
		if (Widget)
		{
			Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}

	void SetTextIfPresent(UTextBlock* Block, const FString& Text)
	{
		if (Block)
		{
			Block->SetText(FText::FromString(Text));
		}
	}

	FString ShipCard(int32 Key, const AAetheriaShipBase* Ship)
	{
		if (!Ship)
		{
			return FString::Printf(TEXT("[%d] —"), Key);
		}
		if (!Ship->IsAlive())
		{
			return FString::Printf(TEXT("[%d] %s — LOST"), Key, *Ship->DisplayName.ToString());
		}
		const int32 HullPct = Ship->MaxHull > 0.0f ? FMath::RoundToInt(100.0f * Ship->Hull / Ship->MaxHull) : 0;
		return FString::Printf(TEXT("[%d] %s  %d%%"), Key, *Ship->GetBarLabel().ToString(), HullPct);
	}
}

void UAetheriaCombatHUD::NativeConstruct()
{
	Super::NativeConstruct();

	using namespace AetheriaHUD;
	SetTextIfPresent(PauseBanner, TEXT("TACTICAL PAUSE — ORDERS EXECUTE ON RESUME [SPACE]"));
	SetTextIfPresent(AimBanner, TEXT("DESIGNATE PRISMATIC BEAM VECTOR — LEFT CLICK"));
	SetTextIfPresent(BeamLabel, TEXT("PRISMATIC STELLAR BEAM [Q]"));
	SetTextIfPresent(ChronoLabel, TEXT("CHRONO SURGE [E]"));
	SetTextIfPresent(ControlsText, TEXT("LMB select · Shift+LMB add · RMB move/attack · SPACE pause · Q beam · E surge · 1–3 groups · WASD pan · Wheel zoom"));

	ShowWidget(PauseBanner, false);
	ShowWidget(AimBanner, false);
	Refresh();
}

void UAetheriaCombatHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Refresh();
}

void UAetheriaCombatHUD::SetPausedBanner(bool bVisible)
{
	AetheriaHUD::ShowWidget(PauseBanner, bVisible);
}

void UAetheriaCombatHUD::SetAimBanner(bool bVisible)
{
	AetheriaHUD::ShowWidget(AimBanner, bVisible);
}

void UAetheriaCombatHUD::Log(E_Speaker Speaker, const FText& Text)
{
	if (!CommsLog)
	{
		return;
	}

	const UAetheriaGameInstance* GI = Cast<UAetheriaGameInstance>(GetGameInstance());
	const FString Name = GI ? GI->SpeakerName(Speaker).ToString() : UEnum::GetDisplayValueAsText(Speaker).ToString();

	UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Line->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Name, *Text.ToString())));
	Line->SetAutoWrapText(true);

	FSlateFontInfo Font = Line->GetFont();
	Font.Size = LogFontSize;
	Line->SetFont(Font);

	CommsLog->AddChild(Line);
	CommsLog->ScrollToEnd();
}

void UAetheriaCombatHUD::Refresh()
{
	using namespace AetheriaHUD;

	const AAetheriaTacticalController* PC = Cast<AAetheriaTacticalController>(GetOwningPlayer());
	const UAetheriaGameInstance* GI = Cast<UAetheriaGameInstance>(GetGameInstance());
	if (!PC)
	{
		return;
	}

	const AAetheriaCombatManager* Manager = PC->Manager;
	const int32 Fleet = Manager ? Manager->GetPhantomFleetStrength() : (GI ? GI->PhantomShipsActive : 0);
	if (GI)
	{
		SetTextIfPresent(StatusText, FString::Printf(TEXT("INSTABILITY %d/100 · PURSUIT %d · PHANTOM FLEET %d/200"),
			FMath::RoundToInt(GI->Instability), GI->PursuitClock, Fleet));
	}

	if (const AAetheriaFlagship* Flagship = PC->Flagship)
	{
		if (FlagshipHullBar)
		{
			FlagshipHullBar->SetPercent(Flagship->MaxHull > 0.0f ? Flagship->Hull / Flagship->MaxHull : 0.0f);
		}
		if (FlagshipShieldBar)
		{
			FlagshipShieldBar->SetPercent(Flagship->MaxShield > 0.0f ? Flagship->Shield / Flagship->MaxShield : 0.0f);
		}

		const float Remaining = Flagship->GetBeamCooldownRemaining();
		if (BeamBar)
		{
			BeamBar->SetPercent(Flagship->BeamCooldown > 0.0f ? 1.0f - Remaining / Flagship->BeamCooldown : 1.0f);
		}

		FString BeamState;
		if (Flagship->bBeamCharging || PC->bBeamQueued)
		{
			BeamState = TEXT("CHARGING");
		}
		else if (Remaining > 0.0f)
		{
			BeamState = FString::Printf(TEXT("%ds"), FMath::CeilToInt(Remaining));
		}
		else
		{
			BeamState = TEXT("READY");
		}
		SetTextIfPresent(BeamStatusText, BeamState);
	}

	if (ChronoBar && GI)
	{
		ChronoBar->SetPercent(GI->ChronoCharge / 100.0f);
	}

	SetTextIfPresent(GroupsText, FString::Printf(TEXT("%s\n%s\n%s"),
		*ShipCard(1, PC->Flagship), *ShipCard(2, PC->Squadron1), *ShipCard(3, PC->Squadron2)));
}
