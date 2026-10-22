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
	CouncilsTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_Councils.DT_Councils")));
	BattlesTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_Battles.DT_Battles")));
}

UDataTable* UAetheriaGameInstance::GetStarNodes() const
{
	return CurrentAct >= 7 ? GetActTable(CurrentAct, TEXT("StarNodes")) : StarNodesTable.LoadSynchronous();
}

UDataTable* UAetheriaGameInstance::GetLeapRoutes() const
{
	return CurrentAct >= 7 ? GetActTable(CurrentAct, TEXT("LeapRoutes")) : LeapRoutesTable.LoadSynchronous();
}

UDataTable* UAetheriaGameInstance::GetNodeEvents() const
{
	return CurrentAct >= 7 ? GetActTable(CurrentAct, TEXT("NodeEvents")) : nullptr;
}

UDataTable* UAetheriaGameInstance::GetActTable(int32 Act, const TCHAR* Kind) const
{
	const FString Name = FString::Printf(TEXT("DT_%s_Act%s"), Kind, *ActNumeral(Act));
	const FSoftObjectPath Path(FString::Printf(TEXT("/Game/Aetheria/Data/%s.%s"), *Name, *Name));
	return Cast<UDataTable>(Path.TryLoad());
}

FString UAetheriaGameInstance::ActNumeral(int32 Act)
{
	switch (Act)
	{
	case 6: return TEXT("VI");
	case 7: return TEXT("VII");
	case 8: return TEXT("VIII");
	case 9: return TEXT("IX");
	case 10: return TEXT("X");
	default: return FString::FromInt(Act);
	}
}

FString UAetheriaGameInstance::ActTitle(int32 Act)
{
	switch (Act)
	{
	case 6: return TEXT("The Far Shore");
	case 7: return TEXT("The Cinder Archive");
	case 8: return TEXT("The Iron Tithe");
	case 9: return TEXT("The Threshold");
	case 10: return TEXT("Gate Zero");
	default: return FString();
	}
}

FName UAetheriaGameInstance::ActStartNode(int32 Act)
{
	switch (Act)
	{
	case 7: return TEXT("EmberRelay");
	case 8: return TEXT("TitheMarker");
	default: return NAME_None;
	}
}

bool UAetheriaGameInstance::HasActData(int32 Act) const
{
	return !ActStartNode(Act).IsNone() && GetActTable(Act, TEXT("StarNodes")) != nullptr;
}

FName UAetheriaGameInstance::ActFlag(const TCHAR* Suffix) const
{
	return FName(*FString::Printf(TEXT("Act%s%s"), *ActNumeral(CurrentAct), Suffix));
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

	const UDataTable* Routes = GetLeapRoutes();
	const FName Route = RouteName(To);
	const FS_LeapRoute* Row = Routes ? Routes->FindRow<FS_LeapRoute>(Route, TEXT("GetLeapCost"), false) : nullptr;
	if (!Row)
	{
		return;
	}

	Found = true;
	Cost = Row->BaseCost;

	// Ash storms (Act VII): leaving a hazardous node costs extra unless the route is charted
	const bool bCharted = ChartedRoutes.Contains(Route);
	if (!bCharted)
	{
		const UDataTable* Nodes = GetStarNodes();
		if (const FS_StarNode* From = Nodes ? Nodes->FindRow<FS_StarNode>(CurrentNode, TEXT("GetLeapCost"), false) : nullptr)
		{
			Cost += From->HazardSurcharge;
		}
	}

	if (bCharted)
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

	if (CurrentAct >= 7)
	{
		FString SupplyReport;
		AdvanceSupplies(SupplyReport);
		if (!SupplyReport.IsEmpty())
		{
			LastReport = FText::FromString(LastReport.ToString() + SupplyReport);
		}
	}

	E_Encounter Encounter = E_Encounter::None;
	const UDataTable* Nodes = GetStarNodes();
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

	// Weave Lane anchors (Act VII): Lyra's first chart action at the Glass Sea and at the Tether Point
	const FName Line = GetState(TEXT("SupplyLine"));
	if (CurrentAct >= 7 && (Line == TEXT("WeaveLane") || Line == TEXT("JointLane"))
		&& (CurrentNode == TEXT("GlassSea") || CurrentNode == TEXT("TetherPoint")))
	{
		const FName Anchor(*(TEXT("Anchor_") + CurrentNode.ToString()));
		if (!StoryFlags.Contains(Anchor))
		{
			StoryFlags.Add(Anchor);
			PursuitClock += 1;
			if (IsSupplyLineReady())
			{
				ModifyLoyalty(E_Speaker::Lyra, 10);
				LastReport = FText::FromString(TEXT("Lyra: both anchors hold. The lane is ours, Captain."));
			}
			else
			{
				LastReport = FText::FromString(TEXT("Lyra: the first anchor holds."));
			}
		}
	}
	OnCampaignStateChanged.Broadcast();
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
	if (StoryFlags.Contains(TEXT("CureSynthesising")))
	{
		LastReport = FText::FromString(TEXT("Dr. Thorne: the lab is occupied by the synthesis. The wounded wait one leap."));
		return;
	}
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

	// Act VII effects
	SeverantStanding = FMath::Clamp(SeverantStanding + Opt.StandingDelta, -100, 100);
	PursuitClock = FMath::Max(0, PursuitClock + Opt.PursuitDelta);
	ArchitectExposure += Opt.ExposureDelta;
	FlagshipHullPct = FMath::Clamp(FlagshipHullPct + Opt.HullDelta, 0.05f, 1.0f);
	for (const FName& Flag : Opt.SetFlags)
	{
		StoryFlags.Add(Flag);
	}
	for (const TPair<FName, FName>& State : Opt.SetStates)
	{
		States.Add(State.Key, State.Value);
	}
	if (Opt.SetFlags.Contains(TEXT("OutpostDark")))
	{
		OutpostDarkLeaps = 2;
	}
	if (Opt.SetFlags.Contains(TEXT("CureDelivered")))
	{
		StoryFlags.Remove(TEXT("CureReady"));
	}

	OnCampaignStateChanged.Broadcast();
}

bool UAetheriaGameInstance::IsOptionAvailable(const FS_CouncilOption& Opt) const
{
	if (Opt.RequiresBackground && Opt.RequiredBackground != Background)
	{
		return false;
	}
	for (const FName& Flag : Opt.RequiredFlags)
	{
		if (!StoryFlags.Contains(Flag))
		{
			return false;
		}
	}
	for (const FName& Flag : Opt.ForbiddenFlags)
	{
		if (StoryFlags.Contains(Flag))
		{
			return false;
		}
	}
	if (SeverantStanding < Opt.MinStanding || SeverantStanding > Opt.MaxStanding)
	{
		return false;
	}
	if (Opt.MinLoyaltyAny.Num() > 0)
	{
		bool bAny = false;
		for (const TPair<E_Speaker, int32>& Need : Opt.MinLoyaltyAny)
		{
			bAny |= CrewLoyalty.FindRef(Need.Key) >= Need.Value;
		}
		if (!bAny)
		{
			return false;
		}
	}
	for (const TPair<FName, FName>& Need : Opt.RequiredStates)
	{
		if (GetState(Need.Key) != Need.Value)
		{
			return false;
		}
	}
	return true;
}

FName UAetheriaGameInstance::GetState(FName Key) const
{
	const FName* Value = States.Find(Key);
	return Value ? *Value : FName(TEXT("None"));
}

void UAetheriaGameInstance::StartAct(int32 Act)
{
	if (!HasActData(Act))
	{
		UE_LOG(LogAetheria, Error, TEXT("StartAct(%d): no chart for Act %s (DT_StarNodes_Act%s)."), Act, *ActNumeral(Act), *ActNumeral(Act));
		return;
	}
	CurrentAct = Act;
	CurrentNode = ActStartNode(Act);
	PursuitClock = 0;
	ChartedRoutes.Reset();
	PendingBattle = NAME_None;
	NextScreen = TEXT("ActStart");
	LastReport = FText::FromString(FString::Printf(TEXT("Act %s: %s."), *ActNumeral(Act), *ActTitle(Act)));
	OnCampaignStateChanged.Broadcast();
}

void UAetheriaGameInstance::StartActVII()
{
	StartAct(7);
}

bool UAetheriaGameInstance::IsSupplyLineReady() const
{
	const FName Line = GetState(TEXT("SupplyLine"));
	if (Line == TEXT("ArchConduit"))
	{
		return true;
	}
	if (Line == TEXT("WeaveLane") || Line == TEXT("JointLane"))
	{
		return StoryFlags.Contains(TEXT("Anchor_GlassSea")) && StoryFlags.Contains(TEXT("Anchor_TetherPoint"));
	}
	return false;
}

bool UAetheriaGameInstance::CanOrderShipment() const
{
	const FName Ordered(*(TEXT("Ordered_") + CurrentNode.ToString()));
	return CurrentAct >= 7 && IsSupplyLineReady() && OutpostDarkLeaps <= 0 && !StoryFlags.Contains(Ordered);
}

void UAetheriaGameInstance::OrderShipment(FName Kind)
{
	if (!CanOrderShipment())
	{
		return;
	}
	StoryFlags.Add(FName(*(TEXT("Ordered_") + CurrentNode.ToString())));

	const FName Line = GetState(TEXT("SupplyLine"));
	if (Line == TEXT("ArchConduit"))
	{
		// through the Architect: immediate, and it counts every crate
		FString Report = TEXT("Arch Conduit delivery.");
		ApplyShipment(Kind, Report);
		ArchitectExposure += 1;
		LastReport = FText::FromString(Report);
	}
	else
	{
		FS_Shipment Shipment;
		Shipment.Kind = Kind;
		Shipment.ArrivesAfterLeaps = Line == TEXT("JointLane") ? 1 : 2;
		PendingShipments.Add(Shipment);
		LastReport = FText::FromString(FString::Printf(TEXT("Outpost One acknowledges. %s shipment in %d leap(s)."), *Kind.ToString(), Shipment.ArrivesAfterLeaps));
	}
	OnCampaignStateChanged.Broadcast();
}

void UAetheriaGameInstance::AdvanceSupplies(FString& Report)
{
	OutpostDarkLeaps = FMath::Max(0, OutpostDarkLeaps - 1);

	for (int32 i = PendingShipments.Num() - 1; i >= 0; --i)
	{
		if (--PendingShipments[i].ArrivesAfterLeaps <= 0)
		{
			ApplyShipment(PendingShipments[i].Kind, Report);
			PendingShipments.RemoveAt(i);
		}
	}

	// the cure: one leap of lab time after the seeds reach a sample
	if (StoryFlags.Contains(TEXT("CureSynthesising")))
	{
		StoryFlags.Remove(TEXT("CureSynthesising"));
		StoryFlags.Add(TEXT("CureReady"));
		Report += TEXT(" Dr. Thorne: the cure is ready.");
	}
}

void UAetheriaGameInstance::ApplyShipment(FName Kind, FString& Report)
{
	if (Kind == TEXT("BioGel"))
	{
		BioGelDoses += 2;
		Report += TEXT(" Bio-Gel crate aboard (+2 doses).");
	}
	else if (Kind == TEXT("Refit"))
	{
		FlagshipHullPct = FMath::Min(1.0f, FlagshipHullPct + 0.2f);
		Report += TEXT(" Dry Dock refit kit installed (+20% hull).");
	}
	else if (Kind == TEXT("Spire"))
	{
		SpireRecalibrations += 1;
		Report += TEXT(" Spire harmonic charge aboard.");
	}
	else if (Kind == TEXT("Seeds"))
	{
		StoryFlags.Add(TEXT("SeedsAboard"));
		Report += TEXT(" Agrarian Seed stock aboard.");
		if (StoryFlags.Contains(TEXT("CureSample")) && !StoryFlags.Contains(TEXT("CureReady")) && !StoryFlags.Contains(TEXT("CureDelivered")))
		{
			StoryFlags.Add(TEXT("CureSynthesising"));
			Report += TEXT(" Dr. Thorne begins synthesis.");
		}
	}
}

FName UAetheriaGameInstance::StandingBand() const
{
	if (SeverantStanding <= -40)
	{
		return TEXT("Hostile");
	}
	return SeverantStanding >= 30 ? FName(TEXT("Trusted")) : FName(TEXT("Wary"));
}

FString UAetheriaGameInstance::CureStatus() const
{
	if (StoryFlags.Contains(TEXT("CureDelivered"))) return TEXT("delivered");
	if (StoryFlags.Contains(TEXT("CureReady"))) return TEXT("ready");
	if (StoryFlags.Contains(TEXT("CureSynthesising"))) return TEXT("synthesising");
	if (StoryFlags.Contains(TEXT("CureSample"))) return TEXT("sample taken");
	return TEXT("none");
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
	case E_Speaker::WardenKesh:     return FText::FromString(TEXT("WARDEN KESH — THE QUIET HOLLOW"));
	case E_Speaker::ExarchIthane:   return FText::FromString(TEXT("EXARCH SOL ITHANE"));
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
	CurrentAct = Defaults->CurrentAct;
	SeverantStanding = Defaults->SeverantStanding;
	States = Defaults->States;
	PendingShipments = Defaults->PendingShipments;
	OutpostDarkLeaps = Defaults->OutpostDarkLeaps;
	PendingBattle = NAME_None;
	NextScreen = TEXT("NewGame");
}
