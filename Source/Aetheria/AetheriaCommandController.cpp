// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCommandController.h"
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
	UseCommandCamera();
	FocusWidget(nullptr);

	if (!GI)
	{
		UE_LOG(LogAetheria, Error, TEXT("PC_Command needs BP_AetheriaGameInstance as the Game Instance."));
		return;
	}

	if (GI->NextScreen == PostCombat)
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
}

void AAetheriaCommandController::OnArrive(E_Encounter Encounter)
{
	using namespace AetheriaFlow;

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

void AAetheriaCommandController::ShowCouncil(UDataTable* Table, FName Key)
{
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

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
