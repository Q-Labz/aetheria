// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaBridgeDirector.h"
#include "Aetheria.h"
#include "Camera/CameraActor.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/RectLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

AAetheriaBridgeDirector::AAetheriaBridgeDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	SpeakerShots.Add(E_Speaker::Captain, TEXT("Shot_Captain"));
	SpeakerShots.Add(E_Speaker::TalaVane, TEXT("Shot_TalaVane"));
	SpeakerShots.Add(E_Speaker::JaxMercer, TEXT("Shot_JaxMercer"));
	SpeakerShots.Add(E_Speaker::ArisThorne, TEXT("Shot_ArisThorne"));
	SpeakerShots.Add(E_Speaker::NixNull, TEXT("Shot_NixNull"));
	SpeakerShots.Add(E_Speaker::VexAI, TEXT("Shot_VexAI"));
	SpeakerShots.Add(E_Speaker::Lyra, TEXT("Shot_Lyra"));
	SpeakerShots.Add(E_Speaker::Cormaker, TEXT("Shot_Cormaker"));
	SpeakerShots.Add(E_Speaker::CoreArchitect, TEXT("Shot_CoreArchitect"));
}

AAetheriaBridgeDirector* AAetheriaBridgeDirector::Find(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (World)
	{
		for (TActorIterator<AAetheriaBridgeDirector> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}

void AAetheriaBridgeDirector::BeginPlay()
{
	Super::BeginPlay();
	InitializeBridge();
}

void AAetheriaBridgeDirector::InitializeBridge()
{
	// PC_Command's BeginPlay may ask for a shot before this actor's BeginPlay runs
	if (bInitialized)
	{
		return;
	}
	bInitialized = true;

	// shot cameras, by every tag starting with Shot_
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		for (const FName& Tag : It->Tags)
		{
			if (Tag.ToString().StartsWith(TEXT("Shot_")))
			{
				Shots.Add(Tag, *It);
			}
		}
	}

	// Vex's column: a dynamic copy of each emissive slot so its glow can rise while Vex speaks
	TArray<AActor*> Columns;
	UGameplayStatics::GetAllActorsWithTag(this, VexColumnTag, Columns);
	for (AActor* Column : Columns)
	{
		TArray<UStaticMeshComponent*> Meshes;
		Column->GetComponents(Meshes);
		for (UStaticMeshComponent* Mesh : Meshes)
		{
			const int32 Slot = Mesh->GetMaterialIndex(TEXT("M_Inlay"));
			if (Slot != INDEX_NONE)
			{
				VexMaterials.Add(Mesh->CreateDynamicMaterialInstance(Slot));
			}
		}
	}
	SetVexSpeaking(false);

	// the bridge's ceiling and cove lights
	for (TActorIterator<ARectLight> It(GetWorld()); It; ++It)
	{
		if (ULightComponent* Light = It->GetLightComponent())
		{
			BridgeLights.Add(Light);
			BaseIntensities.Add(Light->Intensity);
		}
	}

	UE_LOG(LogAetheria, Log, TEXT("Bridge director: %d shots, %d Vex materials, %d lights"), Shots.Num(), VexMaterials.Num(), BridgeLights.Num());
}

ACameraActor* AAetheriaBridgeDirector::FindShot(FName Shot)
{
	InitializeBridge();
	if (const TObjectPtr<ACameraActor>* Found = Shots.Find(Shot))
	{
		return *Found;
	}
	const TObjectPtr<ACameraActor>* Wide = Shots.Find(WideShot);
	return Wide ? Wide->Get() : nullptr;
}

void AAetheriaBridgeDirector::ShowShot(FName Shot)
{
	ACameraActor* Camera = FindShot(Shot);
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!Camera || !PC)
	{
		return;
	}

	// the first cut of a level is immediate; after that, blend
	const bool bFirst = PC->GetViewTarget() == PC->GetPawn() || PC->GetViewTarget() == PC;
	PC->SetViewTargetWithBlend(Camera, bFirst ? 0.0f : BlendTime, VTBlend_Cubic);
}

void AAetheriaBridgeDirector::ShowSpeaker(E_Speaker Speaker)
{
	InitializeBridge();
	const FName* Shot = SpeakerShots.Find(Speaker);
	ShowShot(Shot ? *Shot : WideShot);
	SetVexSpeaking(Speaker == E_Speaker::VexAI);
	SetArchitectSpeaking(Speaker == E_Speaker::CoreArchitect);
}

void AAetheriaBridgeDirector::SetVexSpeaking(bool bSpeaking)
{
	for (UMaterialInstanceDynamic* Material : VexMaterials)
	{
		if (Material)
		{
			Material->SetScalarParameterValue(TEXT("Glow"), bSpeaking ? VexGlow.Y : VexGlow.X);
		}
	}
}

void AAetheriaBridgeDirector::SetArchitectSpeaking(bool bSpeaking)
{
	if (bSpeaking == bArchitectDimmed)
	{
		return;
	}
	bArchitectDimmed = bSpeaking;
	for (int32 i = 0; i < BridgeLights.Num(); ++i)
	{
		if (BridgeLights[i])
		{
			BridgeLights[i]->SetIntensity(BaseIntensities[i] * (bSpeaking ? ArchitectDim : 1.0f));
		}
	}
}
