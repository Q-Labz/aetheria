// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaGameInstance.h"
#include "Engine/DataTable.h"
#include "Aetheria.h"

UAetheriaGameInstance::UAetheriaGameInstance()
{
	CrewLoyalty = {
		{ E_Speaker::TalaVane, 50 },
		{ E_Speaker::JaxMercer, 50 },
		{ E_Speaker::ArisThorne, 50 },
		{ E_Speaker::NixNull, 50 },
		{ E_Speaker::Lyra, 50 },
		{ E_Speaker::Cormaker, 40 }
	};

	LeapRoutesTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_LeapRoutes.DT_LeapRoutes")));
	StarNodesTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_StarNodes.DT_StarNodes")));
}

FName UAetheriaGameInstance::RouteName(FName To) const
{
	return FName(*FString::Printf(TEXT("%s_%s"), *CurrentNode.ToString(), *To.ToString()));
}

void UAetheriaGameInstance::GetLeapCost(FName To, float& Cost, float& Projected, bool& WillOverflow, bool& Found) const
{
	Cost = 0.0f;
	Projected = Instability;
	WillOverflow = false;
	Found = false;

	const UDataTable* Routes = LeapRoutesTable.LoadSynchronous();
	const FName Route = RouteName(To);
	const FS_LeapRoute* Row = Routes ? Routes->FindRow<FS_LeapRoute>(Route, TEXT("GetLeapCost"), false) : nullptr;
	if (!Row)
	{
		return;
	}

	Found = true;
	Cost = Row->BaseCost;

	if (ChartedRoutes.Contains(Route))
	{
		Cost *= 0.5f;
	}

	// Strained containment
	if (Instability >= 50.0f)
	{
		Cost *= 1.25f;
	}

	if (Background == E_Background::GuildEngineer)
	{
		Cost *= 0.9f;
	}

	Projected = Instability + Cost;
	WillOverflow = Projected > OverflowThreshold;
}

void UAetheriaGameInstance::PreviewRupture(float Excess, float& HullLossPct, int32& Injured, int32& Desync) const
{
	const float E = FirstRuptureAbsorbed ? Excess : Excess * 0.5f;

	HullLossPct = 0.08f + E * 0.012f;
	Injured = 1 + FMath::FloorToInt(E / 8.0f);
	Desync = 2 * FMath::FloorToInt(E / 5.0f);
}

E_Encounter UAetheriaGameInstance::ExecuteLeap(FName To)
{
	float Cost, Projected;
	bool bWillOverflow, bFound;
	GetLeapCost(To, Cost, Projected, bWillOverflow, bFound);
	if (!bFound)
	{
		return E_Encounter::None;
	}

	// Leaving from a near-critical drive costs a crew member; the note is kept after the leap report
	FString BleedNote;
	if (Instability >= 80.0f)
	{
		CrewInjured += 1;
		BleedNote = TEXT(" Containment bleed: 1 crew injured.");
	}

	if (bWillOverflow)
	{
		ResolveRupture(Projected - OverflowThreshold);
	}
	else
	{
		Instability = Projected;
		LastReport = FText::FromString(TEXT("Stellar Leap complete. Containment holding."));
	}

	if (!BleedNote.IsEmpty())
	{
		LastReport = FText::FromString(LastReport.ToString() + BleedNote);
	}

	CurrentNode = To;

	E_Encounter Encounter = E_Encounter::None;
	const UDataTable* Nodes = StarNodesTable.LoadSynchronous();
	if (const FS_StarNode* Node = Nodes ? Nodes->FindRow<FS_StarNode>(To, TEXT("ExecuteLeap"), false) : nullptr)
	{
		Encounter = Node->Encounter;
	}
	else
	{
		UE_LOG(LogAetheria, Warning, TEXT("ExecuteLeap: %s is not in DT_StarNodes"), *To.ToString());
	}

	OnCampaignStateChanged.Broadcast();
	return Encounter;
}

void UAetheriaGameInstance::ResolveRupture(float Excess)
{
	float HullLoss;
	int32 Injured, Desync;
	PreviewRupture(Excess, HullLoss, Injured, Desync);

	FString Note;
	if (!FirstRuptureAbsorbed)
	{
		FirstRuptureAbsorbed = true;
		StoryFlags.Add(TEXT("AnomalousDamping"));
		ArchitectExposure += 1;
		Note = TEXT(" Rupture severity reduced by an external harmonic. Origin: unidentified.");
	}

	FlagshipHullPct = FMath::Max(0.15f, FlagshipHullPct - HullLoss);
	CrewInjured += Injured;
	PhantomShipsActive -= Desync;
	Instability = 60.0f;

	LastReport = FText::FromString(FString::Printf(
		TEXT("STAR-DRIVE RUPTURE. Hull integrity reduced %d%%. %d crew injured. %d Phantom vessels desynchronised.%s"),
		FMath::RoundToInt(HullLoss * 100.0f), Injured, Desync, *Note));
}

void UAetheriaGameInstance::ChartRoute(FName To)
{
	const FName Route = RouteName(To);
	if (ChartedRoutes.Contains(Route))
	{
		return;
	}

	ChartedRoutes.Add(Route);
	PursuitClock += 1;
	ModifyLoyalty(E_Speaker::Lyra, 2);
}

void UAetheriaGameInstance::VentChrono()
{
	if (ChronoCharge >= 25.0f)
	{
		ChronoCharge -= 25.0f;
		Instability = FMath::Max(0.0f, Instability - 20.0f);
	}
}

void UAetheriaGameInstance::HoldStation()
{
	Instability = FMath::Max(0.0f, Instability - 10.0f);
	PursuitClock += 1;
}

void UAetheriaGameInstance::SpireRecalibrate()
{
	if (SpireRecalibrations <= 0)
	{
		return;
	}

	SpireRecalibrations -= 1;
	Instability = FMath::Max(0.0f, Instability - 40.0f);
	ArchitectExposure += 1;
	StoryFlags.Add(TEXT("UsedArchConduit"));
	LastReport = FText::FromString(TEXT("Outpost One recalibration received. Note: the signal was relayed through the Omega Arch. The Architect forwarded it without being asked."));
}

void UAetheriaGameInstance::TreatInjured()
{
	if (BioGelDoses > 0 && CrewInjured > 0)
	{
		BioGelDoses -= 1;
		CrewInjured = FMath::Max(0, CrewInjured - 2);
		ModifyLoyalty(E_Speaker::ArisThorne, 2);
	}
}

void UAetheriaGameInstance::ModifyLoyalty(E_Speaker Who, int32 Delta)
{
	const int32 Current = CrewLoyalty.FindRef(Who);
	CrewLoyalty.Add(Who, FMath::Clamp(Current + Delta, -100, 100));
}

void UAetheriaGameInstance::ApplyOption(const FS_CouncilOption& Opt)
{
	ArchitectLeaning = FMath::Clamp(ArchitectLeaning + Opt.LeaningDelta, -100, 100);

	for (const TPair<E_Speaker, int32>& Delta : Opt.LoyaltyDeltas)
	{
		ModifyLoyalty(Delta.Key, Delta.Value);
	}

	Instability = FMath::Clamp(Instability + Opt.InstabilityDelta, 0.0f, 100.0f);
	ChronoCharge = FMath::Clamp(ChronoCharge + Opt.ChronoDelta, 0.0f, 100.0f);

	if (!Opt.SetFlag.IsNone())
	{
		StoryFlags.Add(Opt.SetFlag);
	}

	OnCampaignStateChanged.Broadcast();
}

FName UAetheriaGameInstance::LeaningState() const
{
	if (ArchitectLeaning >= 20)
	{
		return TEXT("Custodian");
	}
	if (ArchitectLeaning <= -20)
	{
		return TEXT("Dominion");
	}
	return TEXT("Undetermined");
}

void UAetheriaGameInstance::ResolveLine(const FS_Line& L, FText& Out, bool& Show) const
{
	Show = L.RequiredFlag.IsNone() || StoryFlags.Contains(L.RequiredFlag);

	const FText* Override = L.BackgroundOverride.Find(Background);
	FString Text = (Override ? *Override : L.Text).ToString();
	Text.ReplaceInline(TEXT("{Captain}"), *CaptainName);
	Text.ReplaceInline(TEXT("{Background}"), *BackgroundTitle());
	Out = FText::FromString(Text);
}

FString UAetheriaGameInstance::BackgroundTitle() const
{
	switch (Background)
	{
	case E_Background::SynodEnvoy:        return TEXT("Synod Envoy");
	case E_Background::GuildEngineer:     return TEXT("Guild Engineer");
	case E_Background::CoalitionVanguard: return TEXT("Coalition Vanguard");
	case E_Background::OutcastCorsair:    return TEXT("Outcast Corsair");
	}
	return FString();
}

FText UAetheriaGameInstance::SpeakerName(E_Speaker S) const
{
	switch (S)
	{
	case E_Speaker::Narrator:       return FText::FromString(TEXT("MISSION LOG"));
	case E_Speaker::Captain:        return FText::FromString(TEXT("CAPTAIN"));
	case E_Speaker::TalaVane:       return FText::FromString(TEXT("TALA VANE — FIRST OFFICER"));
	case E_Speaker::JaxMercer:      return FText::FromString(TEXT("JAX MERCER — CHIEF ENGINEER"));
	case E_Speaker::ArisThorne:     return FText::FromString(TEXT("DR. ARIS THORNE — CMO"));
	case E_Speaker::NixNull:        return FText::FromString(TEXT("NIX \"NULL\" — NEURAL OPS"));
	case E_Speaker::VexAI:          return FText::FromString(TEXT("VEX AI — NAVIGATION"));
	case E_Speaker::Lyra:           return FText::FromString(TEXT("LYRA THE WEAVER"));
	case E_Speaker::Cormaker:       return FText::FromString(TEXT("CMDR. CORMAKER — CORSAIR ARMADA"));
	case E_Speaker::CoreArchitect:  return FText::FromString(TEXT("THE CORE ARCHITECT"));
	case E_Speaker::CommanderVance: return FText::FromString(TEXT("COMMANDER VANCE"));
	}
	return FText::GetEmpty();
}

void UAetheriaGameInstance::ApplyBackgroundStart()
{
	switch (Background)
	{
	case E_Background::GuildEngineer:     ModifyLoyalty(E_Speaker::JaxMercer, 15); break;
	case E_Background::CoalitionVanguard: ModifyLoyalty(E_Speaker::TalaVane, 15); break;
	case E_Background::OutcastCorsair:    ModifyLoyalty(E_Speaker::Cormaker, 20); break;
	case E_Background::SynodEnvoy:        break; // unlocks Rite council options instead
	}
}

void UAetheriaGameInstance::ResetCampaign()
{
	// Defaults come from the class default object, so values set on BP_AetheriaGameInstance are respected
	const UAetheriaGameInstance* Defaults = GetClass()->GetDefaultObject<UAetheriaGameInstance>();

	CaptainName = Defaults->CaptainName;
	Background = Defaults->Background;
	Instability = Defaults->Instability;
	ChronoCharge = Defaults->ChronoCharge;
	FlagshipHullPct = Defaults->FlagshipHullPct;
	CrewInjured = Defaults->CrewInjured;
	BioGelDoses = Defaults->BioGelDoses;
	SpireRecalibrations = Defaults->SpireRecalibrations;
	PhantomShipsActive = Defaults->PhantomShipsActive;
	PursuitClock = Defaults->PursuitClock;
	CurrentNode = Defaults->CurrentNode;
	ChartedRoutes = Defaults->ChartedRoutes;
	StoryFlags = Defaults->StoryFlags;
	ArchitectLeaning = Defaults->ArchitectLeaning;
	ArchitectExposure = Defaults->ArchitectExposure;
	FirstRuptureAbsorbed = Defaults->FirstRuptureAbsorbed;
	CrewLoyalty = Defaults->CrewLoyalty;
	LastReport = Defaults->LastReport;
	NextScreen = TEXT("NewGame");
}
