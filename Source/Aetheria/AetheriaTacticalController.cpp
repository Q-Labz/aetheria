// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaTacticalController.h"
#include "AetheriaCombatHUD.h"
#include "AetheriaCombatManager.h"
#include "AetheriaCommandPawn.h"
#include "AetheriaFlagship.h"
#include "AetheriaGameInstance.h"
#include "AetheriaMoveMarker.h"
#include "AetheriaPhantomSquadron.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace AetheriaTactical
{
	template <typename T>
	T* FindInputAsset(const TCHAR* Path)
	{
		ConstructorHelpers::FObjectFinder<T> Finder(Path);
		return Finder.Succeeded() ? Finder.Object : nullptr;
	}
}

AAetheriaTacticalController::AAetheriaTacticalController()
{
	bShowMouseCursor = true;
	bShouldPerformFullTickWhenPaused = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	using AetheriaTactical::FindInputAsset;
	TacticalContext = FindInputAsset<UInputMappingContext>(TEXT("/Game/Aetheria/Input/IMC_Tactical.IMC_Tactical"));
	SelectAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Select.IA_Select"));
	CommandAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Command.IA_Command"));
	TogglePauseAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_TogglePause.IA_TogglePause"));
	AimBeamAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_AimBeam.IA_AimBeam"));
	ChronoSurgeAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_ChronoSurge.IA_ChronoSurge"));
	Group1Action = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Group1.IA_Group1"));
	Group2Action = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Group2.IA_Group2"));
	Group3Action = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Group3.IA_Group3"));
	CameraPanAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_CameraPan.IA_CameraPan"));
	ZoomAction = FindInputAsset<UInputAction>(TEXT("/Game/Aetheria/Input/IA_Zoom.IA_Zoom"));

	HUDClass = TSoftClassPtr<UAetheriaCombatHUD>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_CombatHUD.WBP_CombatHUD_C")));
	MoveMarkerClass = TSoftClassPtr<AAetheriaMoveMarker>(FSoftObjectPath(TEXT("/Game/Aetheria/Combat/BP_MoveMarker.BP_MoveMarker_C")));
}

void AAetheriaTacticalController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (TacticalContext)
		{
			Subsystem->AddMappingContext(TacticalContext, 0);
		}
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (UClass* Class = HUDClass.LoadSynchronous())
	{
		HUD = CreateWidget<UAetheriaCombatHUD>(this, Class);
		if (HUD)
		{
			HUD->AddToViewport();
		}
	}
}

void AAetheriaTacticalController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		return;
	}

	auto BindStarted = [this, Input](UInputAction* Action, void (AAetheriaTacticalController::*Handler)())
	{
		if (Action)
		{
			Input->BindAction(Action, ETriggerEvent::Started, this, Handler);
		}
	};

	BindStarted(SelectAction, &AAetheriaTacticalController::OnSelect);
	BindStarted(CommandAction, &AAetheriaTacticalController::OnCommand);
	BindStarted(TogglePauseAction, &AAetheriaTacticalController::OnTogglePause);
	BindStarted(AimBeamAction, &AAetheriaTacticalController::OnAimBeam);
	BindStarted(ChronoSurgeAction, &AAetheriaTacticalController::OnChronoSurge);
	BindStarted(Group1Action, &AAetheriaTacticalController::OnGroup1);
	BindStarted(Group2Action, &AAetheriaTacticalController::OnGroup2);
	BindStarted(Group3Action, &AAetheriaTacticalController::OnGroup3);

	if (CameraPanAction)
	{
		Input->BindAction(CameraPanAction, ETriggerEvent::Triggered, this, &AAetheriaTacticalController::OnCameraPan);
	}
	if (ZoomAction)
	{
		Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AAetheriaTacticalController::OnZoom);
	}
}

void AAetheriaTacticalController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bAimingBeam && Flagship)
	{
		Flagship->ShowPreview(CursorToPlane());
	}
}

FVector AAetheriaTacticalController::CursorToPlane() const
{
	FVector Origin, Direction;
	if (!DeprojectMousePositionToWorld(Origin, Direction) || FMath::IsNearlyZero(Direction.Z))
	{
		return FVector::ZeroVector;
	}

	return FMath::LinePlaneIntersection(Origin, Origin + Direction * 1000000.0f, FVector::ZeroVector, FVector::UpVector);
}

AAetheriaShipBase* AAetheriaTacticalController::ShipUnderCursor() const
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		AAetheriaShipBase* Ship = Cast<AAetheriaShipBase>(Hit.GetActor());
		if (Ship && Ship->IsAlive())
		{
			return Ship;
		}
	}
	return nullptr;
}

void AAetheriaTacticalController::SetTacPause(bool bPaused)
{
	bTacPaused = bPaused;
	UGameplayStatics::SetGamePaused(this, bPaused);

	if (HUD)
	{
		HUD->SetPausedBanner(bPaused);
	}
	if (!bPaused)
	{
		ExecuteQueued();
	}
}

void AAetheriaTacticalController::ExecuteQueued()
{
	if (bBeamQueued)
	{
		bBeamQueued = false;
		if (Flagship)
		{
			Flagship->FireBeam(QueuedBeamTarget);
			Flagship->HidePreview();
		}
	}

	if (bSurgeQueued)
	{
		bSurgeQueued = false;
		if (Manager)
		{
			Manager->StartSurge();
		}
	}
}

void AAetheriaTacticalController::Log(E_Speaker Speaker, const FString& Text)
{
	if (HUD)
	{
		HUD->Log(Speaker, FText::FromString(Text));
	}
}

void AAetheriaTacticalController::SelectShips(const TArray<AAetheriaShipBase*>& Ships)
{
	for (AAetheriaShipBase* Ship : Selected)
	{
		if (Ship)
		{
			Ship->SetSelected(false);
		}
	}

	Selected.Reset();
	for (AAetheriaShipBase* Ship : Ships)
	{
		if (Ship && Ship->IsAlive() && Ship->Faction == E_Faction::Player)
		{
			Selected.AddUnique(Ship);
			Ship->SetSelected(true);
		}
	}
}

void AAetheriaTacticalController::PruneSelection()
{
	Selected.RemoveAll([](const AAetheriaShipBase* Ship) { return !Ship || !Ship->IsAlive(); });
}

void AAetheriaTacticalController::OnTogglePause()
{
	SetTacPause(!bTacPaused);
	Log(E_Speaker::VexAI, bTacPaused ? TEXT("Tactical pause. Fleet awaiting orders.") : TEXT("Resuming. Orders executing."));
}

void AAetheriaTacticalController::OnSelect()
{
	if (bAimingBeam)
	{
		AAetheriaShipBase* Target = ShipUnderCursor();
		const FVector Point = (Target && Flagship && Flagship->IsHostileTo(Target)) ? Target->GetActorLocation() : CursorToPlane();

		bAimingBeam = false;
		if (HUD)
		{
			HUD->SetAimBanner(false);
		}
		if (!Flagship)
		{
			return;
		}

		if (bTacPaused)
		{
			bBeamQueued = true;
			QueuedBeamTarget = Point;
			Flagship->ShowPreview(Point);
			Log(E_Speaker::TalaVane, TEXT("Firing solution locked. Prismatic Stellar Beam will discharge on resume."));
		}
		else
		{
			Flagship->HidePreview();
			Flagship->FireBeam(Point);
		}
		return;
	}

	AAetheriaShipBase* Ship = ShipUnderCursor();
	if (Ship && Ship->Faction == E_Faction::Player)
	{
		const bool bAdd = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
		TArray<AAetheriaShipBase*> NewSelection;
		if (bAdd)
		{
			for (AAetheriaShipBase* Existing : Selected)
			{
				NewSelection.Add(Existing);
			}
		}
		NewSelection.Add(Ship);
		SelectShips(NewSelection);
	}
	else
	{
		SelectShips({});
	}
}

void AAetheriaTacticalController::OnCommand()
{
	PruneSelection();
	if (Selected.Num() == 0)
	{
		return;
	}

	AAetheriaShipBase* Target = ShipUnderCursor();
	if (Target && Target->Faction != E_Faction::Player)
	{
		for (AAetheriaShipBase* Ship : Selected)
		{
			Ship->IssueAttack(Target);
		}
		Log(E_Speaker::TalaVane, FString::Printf(TEXT("Engaging %s."), *Target->DisplayName.ToString()));
		return;
	}

	const FVector Dest = CursorToPlane();

	FVector Average = FVector::ZeroVector;
	for (const AAetheriaShipBase* Ship : Selected)
	{
		Average += Ship->GetActorLocation();
	}
	Average /= Selected.Num();

	// spread the group across the line of advance
	FVector Right = FVector::CrossProduct((Dest - Average).GetSafeNormal2D(), FVector::UpVector);
	if (Right.IsNearlyZero())
	{
		Right = FVector::RightVector;
	}

	const float Centre = (Selected.Num() - 1) * 0.5f;
	for (int32 Index = 0; Index < Selected.Num(); ++Index)
	{
		Selected[Index]->IssueMove(Dest + Right * ((Index - Centre) * FormationSpacing));
	}

	if (UClass* MarkerClass = MoveMarkerClass.LoadSynchronous())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AAetheriaMoveMarker>(MarkerClass, FTransform(Dest), Params);
	}
}

void AAetheriaTacticalController::OnAimBeam()
{
	if (!Flagship || !Flagship->IsAlive())
	{
		return;
	}

	if (Flagship->CanFireBeam() && !bBeamQueued)
	{
		bAimingBeam = true;
		if (HUD)
		{
			HUD->SetAimBanner(true);
		}
		Log(E_Speaker::TalaVane, TEXT("Designate beam vector. Left-click to confirm."));
	}
	else
	{
		const int32 Seconds = FMath::CeilToInt(Flagship->GetBeamCooldownRemaining());
		Log(E_Speaker::JaxMercer, Flagship->bBeamCharging || bBeamQueued
			? FString(TEXT("Prismatic array is already committed, Captain."))
			: FString::Printf(TEXT("Prismatic array recharging. %d seconds."), Seconds));
	}
}

void AAetheriaTacticalController::OnChronoSurge()
{
	const UAetheriaGameInstance* GI = Cast<UAetheriaGameInstance>(GetGameInstance());
	if (!GI || GI->ChronoCharge < 30.0f || (Manager && Manager->bSurgeActive) || bSurgeQueued)
	{
		Log(E_Speaker::JaxMercer, TEXT("Insufficient Chrono charge, Captain."));
		return;
	}

	if (bTacPaused)
	{
		bSurgeQueued = true;
		Log(E_Speaker::JaxMercer, TEXT("Chrono-Core primed. Surge will engage on resume."));
	}
	else if (Manager)
	{
		Manager->StartSurge();
	}
}

void AAetheriaTacticalController::OnGroup1()
{
	SelectShips({ Flagship.Get() });
}

void AAetheriaTacticalController::OnGroup2()
{
	SelectShips({ Squadron1.Get() });
}

void AAetheriaTacticalController::OnGroup3()
{
	SelectShips({ Squadron2.Get() });
}

void AAetheriaTacticalController::OnCameraPan(const FInputActionValue& Value)
{
	if (AAetheriaCommandPawn* CommandPawn = Cast<AAetheriaCommandPawn>(GetPawn()))
	{
		CommandPawn->Pan(Value.Get<FVector2D>());
	}
}

void AAetheriaTacticalController::OnZoom(const FInputActionValue& Value)
{
	if (AAetheriaCommandPawn* CommandPawn = Cast<AAetheriaCommandPawn>(GetPawn()))
	{
		CommandPawn->Zoom(Value.Get<float>());
	}
}
