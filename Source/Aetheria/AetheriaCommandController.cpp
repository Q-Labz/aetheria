// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCommandController.h"
#include "AetheriaBridgeDirector.h"
#include "AetheriaCommandWidgets.h"
#include "AetheriaGameInstance.h"
#include "Aetheria.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraActor.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

namespace AetheriaFlow
{
	const FName NewGame(TEXT("NewGame"));
	const FName PostCombat(TEXT("PostCombat"));
	const FName StarMap(TEXT("StarMap"));

	const FName ToMap(TEXT("ToMap"));
	const FName ToCombat(TEXT("ToCombat"));
	const FName AfterCombat(TEXT("AfterCombat"));
	const FName AfterRelay(TEXT("AfterRelay"));
	const FName AfterReveal(TEXT("AfterReveal"));
	const FName AfterDirective(TEXT("AfterDirective"));
	const FName End(TEXT("End"));

	const FName Relay(TEXT("Relay"));
	const FName Directive(TEXT("Directive"));
	const FName EmberRelay(TEXT("EmberRelay"));
	const FName LanternCleared(TEXT("LanternCleared"));
	const FName CameraTag(TEXT("CommandCamera"));
}

AAetheriaCommandController::AAetheriaCommandController()
{
	bShowMouseCursor = true;

	CreationClass = TSoftClassPtr<UAetheriaCaptainCreation>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_CaptainCreation.WBP_CaptainCreation_C")));
	DialogueClass = TSoftClassPtr<UAetheriaDialogue>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_Dialogue.WBP_Dialogue_C")));
	StarMapClass = TSoftClassPtr<UAetheriaStarMap>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_StarMap.WBP_StarMap_C")));
	CouncilClass = TSoftClassPtr<UAetheriaCouncil>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_Council.WBP_Council_C")));
	SliceEndClass = TSoftClassPtr<UAetheriaSliceEnd>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_SliceEnd.WBP_SliceEnd_C")));

	SequencesTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_Sequences.DT_Sequences")));
	RelayTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_Council_Relay.DT_Council_Relay")));
	DirectiveTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Aetheria/Data/DT_Council_Directive.DT_Council_Directive")));

	CombatLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Aetheria/Maps/L_Combat_LanternDrift.L_Combat_LanternDrift")));
	CommandLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Aetheria/Maps/L_Command.L_Command")));
}

void AAetheriaCommandController::BeginPlay()
{
	Super::BeginPlay();

	using namespace AetheriaFlow;

	GI = Cast<UAetheriaGameInstance>(GetGameInstance());
	Director = AAetheriaBridgeDirector::Find(this);
	UseCommandCamera();
	FocusWidget(nullptr);

	if (!GI)
	{
		UE_LOG(LogAetheria, Error, TEXT("PC_Command needs BP_AetheriaGameInstance as the Game Instance."));
		return;
	}

	if (GI->CurrentAct >= 7 && GI->NextScreen == TEXT("ActStart"))
	{
		GI->NextScreen = StarMap;
		PlaySequence(FName(*FString::Printf(TEXT("Act%s_Opening"), *UAetheriaGameInstance::ActNumeral(GI->CurrentAct))), TEXT("ToNode"));
	}
	else if (GI->CurrentAct >= 7 && GI->NextScreen == PostCombat)
	{
		AfterDataBattle();
	}
	else if (GI->NextScreen == PostCombat)
	{
		PlaySequence(TEXT("PostCombat"), AfterCombat);
	}
	else if (GI->NextScreen == StarMap)
	{
		ShowStarMap();
	}
	else
	{
		ShowCreation();
	}
}

void AAetheriaCommandController::UseCommandCamera()
{
	if (Director)
	{
		Director->ShowShot(Director->WideShot);
		return;
	}

	TArray<AActor*> Cameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ACameraActor::StaticClass(), AetheriaFlow::CameraTag, Cameras);
	if (Cameras.Num() == 0)
	{
		UGameplayStatics::GetAllActorsOfClass(this, ACameraActor::StaticClass(), Cameras);
	}
	if (Cameras.Num() > 0)
	{
		SetViewTarget(Cameras[0]);
	}
}

void AAetheriaCommandController::FocusWidget(UUserWidget* Widget)
{
	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (Widget)
	{
		Mode.SetWidgetToFocus(Widget->TakeWidget());
	}
	SetInputMode(Mode);
}

void AAetheriaCommandController::ShowCreation()
{
	ShowShot(Director ? Director->WideShot : NAME_None);
	if (UClass* Class = CreationClass.LoadSynchronous())
	{
		if (UAetheriaCaptainCreation* Creation = CreateWidget<UAetheriaCaptainCreation>(this, Class))
		{
			Creation->AddToViewport();
			FocusWidget(Creation);
		}
	}
}

void AAetheriaCommandController::ShowStarMap()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

	if (GI)
	{
		GI->NextScreen = AetheriaFlow::StarMap;
	}
	ShowShot(Director ? Director->ChartShot : NAME_None);

	if (UClass* Class = StarMapClass.LoadSynchronous())
	{
		if (UAetheriaStarMap* Map = CreateWidget<UAetheriaStarMap>(this, Class))
		{
			Map->AddToViewport();
			FocusWidget(Map);
		}
	}
}

void AAetheriaCommandController::PlaySequence(FName Row, FName AfterKey)
{
	const UDataTable* Table = SequencesTable.LoadSynchronous();
	const FS_Sequence* Sequence = Table ? Table->FindRow<FS_Sequence>(Row, TEXT("PlaySequence"), false) : nullptr;
	if (!Sequence)
	{
		UE_LOG(LogAetheria, Error, TEXT("PlaySequence: DT_Sequences has no row %s"), *Row.ToString());
		AfterDialogue = AfterKey;
		OnSequenceDone();
		return;
	}

	PlayLines(Sequence->Lines, AfterKey);
}

void AAetheriaCommandController::PlayLines(const TArray<FS_Line>& Lines, FName AfterKey)
{
	UClass* Class = DialogueClass.LoadSynchronous();
	UAetheriaDialogue* Dialogue = Class ? CreateWidget<UAetheriaDialogue>(this, Class) : nullptr;
	AfterDialogue = AfterKey;
	if (!Dialogue)
	{
		OnSequenceDone();
		return;
	}

	Dialogue->SetLines(Lines);
	Dialogue->OnDialogueFinished.AddDynamic(this, &AAetheriaCommandController::OnSequenceDone);
	Dialogue->OnLineShown.AddDynamic(this, &AAetheriaCommandController::OnLineShown);
	Dialogue->AddToViewport(5);
	FocusWidget(Dialogue);
}

void AAetheriaCommandController::OnSequenceDone()
{
	using namespace AetheriaFlow;

	const FName After = AfterDialogue;
	AfterDialogue = NAME_None;

	if (After == ToMap)
	{
		ShowStarMap();
	}
	else if (After == ToCombat)
	{
		if (GI)
		{
			GI->NextScreen = PostCombat;
		}
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, CombatLevel);
	}
	else if (After == AfterCombat)
	{
		if (GI && GI->CurrentNode == EmberRelay)
		{
			ShowCouncil(GetRelayTable(), Relay);
		}
		else
		{
			ShowStarMap();
		}
	}
	else if (After == AfterRelay)
	{
		PlaySequence(TEXT("ArchitectReveal"), AfterReveal);
	}
	else if (After == AfterReveal)
	{
		ShowCouncil(GetDirectiveTable(), Directive);
	}
	else if (After == AfterDirective)
	{
		ApplyArchitectConsequence();
		PlaySequence(FName(*(TEXT("Consequence_") + (GI ? GI->LeaningState().ToString() : FString(TEXT("Undetermined"))))), End);
	}
	else if (After == End)
	{
		ShowSliceEnd();
	}
	else if (After == TEXT("ToNode"))
	{
		RunNode(GI ? GI->CurrentNode : NAME_None);
	}
	else if (After == TEXT("AfterArrival"))
	{
		ContinueNode();
	}
	else if (After == TEXT("CouncilIntro"))
	{
		const FName Council = PendingNextCouncil;
		PendingNextCouncil = NAME_None;
		ShowDataCouncil(Council);
	}
	else if (After == TEXT("AfterCouncil"))
	{
		if (!PendingStartBattle.IsNone())
		{
			const FName Battle = PendingStartBattle;
			PendingStartBattle = NAME_None;
			StartBattle(Battle);
		}
		else if (!PendingNextCouncil.IsNone() && CanHoldCouncil(PendingNextCouncil))
		{
			const FName Council = PendingNextCouncil;
			PendingNextCouncil = NAME_None;
			ShowDataCouncil(Council);
		}
		else
		{
			PendingNextCouncil = NAME_None;
			ContinueNode();
		}
	}
	else if (After == TEXT("ToBattle"))
	{
		const FName Battle = PendingStartBattle;
		PendingStartBattle = NAME_None;
		StartBattle(Battle);
	}
	else if (After == TEXT("AfterDebrief"))
	{
		if (GI && GI->StoryFlags.Contains(GI->ActFlag(TEXT("FinaleWon"))) && !GI->StoryFlags.Contains(GI->ActFlag(TEXT("Complete"))))
		{
			PlayActEnd();
		}
		else
		{
			ContinueNode();
		}
	}
	else if (After == TEXT("AfterRevelation"))
	{
		PlayActReport();
	}
}

bool AAetheriaCommandController::CanHoldCouncil(FName Council) const
{
	if (!GI || Council.IsNone() || GI->StoryFlags.Contains(FName(*(TEXT("Council_") + Council.ToString()))))
	{
		return false;
	}
	const UDataTable* Defs = GI->CouncilsTable.LoadSynchronous();
	const FS_CouncilDef* Def = Defs ? Defs->FindRow<FS_CouncilDef>(Council, TEXT("CanHoldCouncil"), false) : nullptr;
	if (!Def)
	{
		return false;
	}
	for (const FName& Flag : Def->RequiredFlags)
	{
		if (!GI->StoryFlags.Contains(Flag))
		{
			return false;
		}
	}
	const UDataTable* Options = Def->Options.LoadSynchronous();
	bool bAny = false;
	if (Options)
	{
		Options->ForeachRow<FS_CouncilOption>(TEXT("CanHoldCouncil"), [this, &bAny](const FName&, const FS_CouncilOption& Option)
		{
			bAny |= GI->IsOptionAvailable(Option);
		});
	}
	return bAny;
}

void AAetheriaCommandController::StartActVII()
{
	StartAct(7);
}

void AAetheriaCommandController::StartAct(int32 Act)
{
	if (!GI || !GI->HasActData(Act))
	{
		UE_LOG(LogAetheria, Error, TEXT("StartAct(%d): the act has no chart yet."), Act);
		return;
	}
	GI->StartAct(Act);
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, CommandLevel);
}

void AAetheriaCommandController::StartNextAct()
{
	if (GI)
	{
		StartAct(GI->CurrentAct < 7 ? 7 : GI->CurrentAct + 1);
	}
}

void AAetheriaCommandController::RunNode(FName Node)
{
	if (!GI)
	{
		return;
	}
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

	// Iron Tithe ambush: the pursuit caught up (Act VII keeps its original names: AmbushSprung, Ambush, Ambush_Intro)
	const bool bActVII = GI->CurrentAct == 7;
	const FString ActTag = TEXT("_Act") + UAetheriaGameInstance::ActNumeral(GI->CurrentAct);
	const FName AmbushFlag(*(TEXT("AmbushSprung") + (bActVII ? FString() : ActTag)));
	if (GI->PursuitClock >= 5 && !GI->StoryFlags.Contains(AmbushFlag))
	{
		GI->StoryFlags.Add(AmbushFlag);
		GI->PursuitClock = 0;
		PendingStartBattle = FName(*(TEXT("Ambush") + (bActVII ? FString() : ActTag)));
		PlaySequence(FName(*(TEXT("Ambush") + (bActVII ? FString() : ActTag) + TEXT("_Intro"))), TEXT("ToBattle"));
		return;
	}

	const UDataTable* Events = GI->GetNodeEvents();
	const FS_NodeEvent* Event = Events ? Events->FindRow<FS_NodeEvent>(Node, TEXT("RunNode"), false) : nullptr;
	const FName Visited(*(TEXT("Visited_") + Node.ToString()));
	if (Event && !GI->StoryFlags.Contains(Visited))
	{
		GI->StoryFlags.Add(Visited);
		for (const FName& Flag : Event->ArrivalFlags)
		{
			GI->StoryFlags.Add(Flag);
		}
		if (!Event->ArrivalSequence.IsNone())
		{
			PlaySequence(Event->ArrivalSequence, TEXT("AfterArrival"));
			return;
		}
	}
	ContinueNode();
}

void AAetheriaCommandController::ContinueNode()
{
	if (!GI)
	{
		return;
	}
	const UDataTable* Events = GI->GetNodeEvents();
	if (const FS_NodeEvent* Event = Events ? Events->FindRow<FS_NodeEvent>(GI->CurrentNode, TEXT("ContinueNode"), false) : nullptr)
	{
		for (const FName& Council : Event->Councils)
		{
			if (CanHoldCouncil(Council))
			{
				const UDataTable* Defs = GI->CouncilsTable.LoadSynchronous();
				const FS_CouncilDef* Def = Defs ? Defs->FindRow<FS_CouncilDef>(Council, TEXT("ContinueNode"), false) : nullptr;
				if (Def && !Def->Intro.IsNone())
				{
					PendingNextCouncil = Council;
					PlaySequence(Def->Intro, TEXT("CouncilIntro"));
				}
				else
				{
					ShowDataCouncil(Council);
				}
				return;
			}
		}
	}
	ShowStarMap();
}

void AAetheriaCommandController::ShowDataCouncil(FName Council)
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
	ShowShot(Director ? Director->CouncilShot : NAME_None);

	if (UClass* Class = CouncilClass.LoadSynchronous())
	{
		if (UAetheriaCouncil* Widget = CreateWidget<UAetheriaCouncil>(this, Class))
		{
			Widget->SetupDef(Council);
			Widget->AddToViewport();
			FocusWidget(Widget);
		}
	}
}

void AAetheriaCommandController::OnCouncilChosen(FName Council, const FS_CouncilOption& Opt)
{
	if (GI)
	{
		GI->StoryFlags.Add(FName(*(TEXT("Council_") + Council.ToString())));
	}
	PendingNextCouncil = Opt.NextCouncil;
	PendingStartBattle = Opt.StartBattle;
	PlayLines(Opt.Outcome, TEXT("AfterCouncil"));
}

void AAetheriaCommandController::StartBattle(FName Battle)
{
	if (GI)
	{
		GI->PendingBattle = Battle;
		GI->NextScreen = AetheriaFlow::PostCombat;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, CombatLevel);
}

void AAetheriaCommandController::AfterDataBattle()
{
	const FName Battle = GI ? GI->PendingBattle : NAME_None;
	if (GI)
	{
		GI->PendingBattle = NAME_None;
		GI->NextScreen = AetheriaFlow::StarMap;
	}
	const UDataTable* Battles = GI ? GI->BattlesTable.LoadSynchronous() : nullptr;
	const FS_Battle* Row = Battles ? Battles->FindRow<FS_Battle>(Battle, TEXT("AfterDataBattle"), false) : nullptr;
	if (Row && !Row->Debrief.IsNone())
	{
		PlaySequence(Row->Debrief, TEXT("AfterDebrief"));
	}
	else
	{
		AfterDialogue = TEXT("AfterDebrief");
		OnSequenceDone();
	}
}

void AAetheriaCommandController::PlayActEnd()
{
	if (!GI)
	{
		return;
	}
	if (GI->CurrentAct != 7)
	{
		PlayActReport();
		return;
	}
	const FName State = GI->LeaningState();
	const FName Revelation = State == TEXT("Custodian") ? FName(TEXT("Guardian"))
		: State == TEXT("Dominion") ? FName(TEXT("Warden")) : FName(TEXT("Unresolved"));
	GI->States.Add(TEXT("ArchitectRevelation2"), Revelation);
	PlaySequence(FName(*(TEXT("Revelation_") + Revelation.ToString())), TEXT("AfterRevelation"));
}

void AAetheriaCommandController::PlayActReport()
{
	using namespace AetheriaFlow;

	if (!GI)
	{
		return;
	}
	FString Text;
	if (GI->CurrentAct == 7)
	{
		Text = FString::Printf(
			TEXT("MISSION REPORT — THE CINDER ARCHIVE. The Vault of Records is %s. Severant standing: %s. The Silence: %s. Supply line: %s. Phantom Fleet strength: %d of 200. Commander Vance's fleet withdrew toward Tithe Anchorage. Report ends."),
			*GI->GetState(TEXT("ArchiveState")).ToString(), *GI->StandingBand().ToString(),
			GI->StoryFlags.Contains(TEXT("CureDelivered")) ? TEXT("cured") : TEXT("untreated"),
			*GI->GetState(TEXT("SupplyLine")).ToString(), GI->PhantomShipsActive);
	}
	else if (GI->CurrentAct == 8)
	{
		const FName Fate = GI->GetState(TEXT("VanceFate"));
		const TCHAR* FateText = Fate == TEXT("Tried") ? TEXT("tried before the fleet")
			: Fate == TEXT("Absorbed") ? TEXT("held; the Remnant absorbed into the fleet")
			: Fate == TEXT("Severants") ? TEXT("handed to the Severant council")
			: Fate == TEXT("Released") ? TEXT("released with his crews, unarmed") : TEXT("unresolved");
		const FName Forge = GI->GetState(TEXT("GraftForgeState"));
		const FName Allies = GI->GetState(TEXT("SeverantAllies"));
		Text = FString::Printf(
			TEXT("MISSION REPORT — THE IRON TITHE. Tithe Anchorage is in fleet hands. Commander Vance: %s. The Graft Forge: %s. Severant vessels standing with the fleet: %s. The Corsair Armada: %s. Phantom Fleet strength: %d of 200. Report ends."),
			FateText, Forge == TEXT("None") ? TEXT("intact") : *Forge.ToString().ToLower(),
			Allies == TEXT("None") ? TEXT("0") : *Allies.ToString(),
			GI->StoryFlags.Contains(TEXT("CorsairsDeparted")) ? TEXT("departed for Rustmarch") : TEXT("with the fleet"),
			GI->PhantomShipsActive);
	}
	else
	{
		Text = FString::Printf(TEXT("MISSION REPORT — %s. Phantom Fleet strength: %d of 200. Report ends."),
			*UAetheriaGameInstance::ActTitle(GI->CurrentAct).ToUpper(), GI->PhantomShipsActive);
	}

	TArray<FS_Line> Report;
	FS_Line Line;
	Line.Speaker = E_Speaker::Narrator;
	Line.Text = FText::FromString(Text);
	Report.Add(Line);
	GI->StoryFlags.Add(GI->ActFlag(TEXT("Complete")));
	PlayLines(Report, End);
}

void AAetheriaCommandController::OnArrive(E_Encounter Encounter)
{
	using namespace AetheriaFlow;

	if (GI && GI->CurrentAct >= 7)
	{
		RunNode(GI->CurrentNode);
		return;
	}

	const bool bCleared = GI && GI->StoryFlags.Contains(LanternCleared);
	const bool bAtRelay = GI && GI->CurrentNode == EmberRelay;

	if (!bCleared && Encounter == E_Encounter::Combat)
	{
		PlaySequence(TEXT("CombatIntro"), ToCombat);
	}
	else if (!bCleared && bAtRelay)
	{
		PlaySequence(TEXT("VanceIntercept"), ToCombat);
	}
	else if (bCleared && bAtRelay)
	{
		ShowCouncil(GetRelayTable(), Relay);
	}
	else
	{
		ShowStarMap();
	}
}

void AAetheriaCommandController::OnLineShown(E_Speaker Speaker)
{
	if (Director)
	{
		Director->ShowSpeaker(Speaker);
	}
}

void AAetheriaCommandController::ShowShot(FName Shot)
{
	if (Director && !Shot.IsNone())
	{
		Director->ShowShot(Shot);
	}
}

void AAetheriaCommandController::ShowCouncil(UDataTable* Table, FName Key)
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
	ShowShot(Director ? Director->CouncilShot : NAME_None);

	if (UClass* Class = CouncilClass.LoadSynchronous())
	{
		if (UAetheriaCouncil* Council = CreateWidget<UAetheriaCouncil>(this, Class))
		{
			Council->Setup(Table, Key);
			Council->AddToViewport();
			FocusWidget(Council);
		}
	}
}

void AAetheriaCommandController::ApplyArchitectConsequence()
{
	if (!GI)
	{
		return;
	}

	const FName State = GI->LeaningState();
	if (State == TEXT("Custodian"))
	{
		GI->Instability = FMath::Max(0.0f, GI->Instability - 30.0f);
		GI->SpireRecalibrations += 1;
	}
	else if (State == TEXT("Dominion"))
	{
		GI->PhantomShipsActive -= 20;
		GI->StoryFlags.Add(TEXT("SquadronIIIAlienated"));
	}
	else
	{
		GI->StoryFlags.Add(TEXT("ArchitectSilent"));
	}
	GI->OnCampaignStateChanged.Broadcast();
}

void AAetheriaCommandController::ShowSliceEnd()
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
	ShowShot(Director ? Director->WideShot : NAME_None);

	if (UClass* Class = SliceEndClass.LoadSynchronous())
	{
		if (UAetheriaSliceEnd* SliceEnd = CreateWidget<UAetheriaSliceEnd>(this, Class))
		{
			SliceEnd->AddToViewport();
			FocusWidget(SliceEnd);
		}
	}
}

void AAetheriaCommandController::NewCommand()
{
	if (GI)
	{
		GI->ResetCampaign();
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, CommandLevel);
}
