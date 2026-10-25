// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaNodeScenery.h"
#include "Aetheria.h"
#include "AetheriaGameInstance.h"
#include "Components/LightComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
	const FString NodeTagPrefix(TEXT("Node_"));
}

AAetheriaNodeScenery::AAetheriaNodeScenery()
{
	PrimaryActorTick.bCanEverTick = false;

	// the light of each node's star (WORLD_BIBLE.md 3.2), kept pale so ship hulls keep their own colours
	NodeLightColors.Add(TEXT("ArchWake"), FLinearColor(0.78f, 0.86f, 1.0f));       // Threnody, blue-white
	NodeLightColors.Add(TEXT("LanternDrift"), FLinearColor(0.72f, 1.0f, 0.90f));   // Lantern Mother's teal
	NodeLightColors.Add(TEXT("EmberRelay"), FLinearColor(1.0f, 0.70f, 0.48f));     // Cinderwake, ember
	NodeLightColors.Add(TEXT("AshfallVerge"), FLinearColor(1.0f, 0.82f, 0.62f));   // light through ash
	NodeLightColors.Add(TEXT("GlassSea"), FLinearColor(0.82f, 0.95f, 1.0f));       // glare off the glass
	NodeLightColors.Add(TEXT("TetherPoint"), FLinearColor(0.80f, 0.74f, 1.0f));    // lensed starlight
	NodeLightColors.Add(TEXT("CinderArchive"), FLinearColor(1.0f, 0.92f, 0.80f));  // pale ash-gold
	NodeLightColors.Add(TEXT("QuietHollow"), FLinearColor(1.0f, 0.86f, 0.72f));    // the refuge's warm lamps
	NodeLightColors.Add(TEXT("PyreGate"), FLinearColor(0.86f, 0.92f, 1.0f));       // the Vault shaft's white
	NodeLightColors.Add(TEXT("TitheMarker"), FLinearColor(1.0f, 0.66f, 0.60f));    // the Anchorage's red light
	NodeLightColors.Add(TEXT("VaultOfRecords"), FLinearColor(0.80f, 0.90f, 1.0f)); // the polar shaft
	NodeLightColors.Add(TEXT("Grieve"), FLinearColor(0.92f, 0.80f, 0.72f)); // Grieve's rust-banded glow
	NodeLightColors.Add(TEXT("Rustmarch"), FLinearColor(1.0f, 0.74f, 0.45f)); // the small orange star
	NodeLightColors.Add(TEXT("SerenVael"), FLinearColor(0.92f, 0.96f, 1.0f)); // salt-white glare
	NodeLightColors.Add(TEXT("MeridianYards"), FLinearColor(0.78f, 0.96f, 1.0f)); // crystal veins of the yard ring
	NodeLightColors.Add(TEXT("Meridian"), FLinearColor(0.80f, 0.95f, 1.0f)); // the ecumenopolis lattice
	NodeLightColors.Add(TEXT("ForgeEcho"), FLinearColor(1.0f, 0.78f, 0.52f)); // the accretion disc
	NodeLightColors.Add(TEXT("TheHush"), FLinearColor(0.70f, 0.62f, 1.0f)); // the Listener's shimmer
	NodeLightColors.Add(TEXT("Vigil"), FLinearColor(0.68f, 0.82f, 1.0f)); // Silence blue
	NodeLightColors.Add(TEXT("GateZero"), FLinearColor(0.82f, 0.74f, 0.95f)); // the seal plates' glow
}

void AAetheriaNodeScenery::BeginPlay()
{
	Super::BeginPlay();

	GI = Cast<UAetheriaGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->OnCampaignStateChanged.AddDynamic(this, &AAetheriaNodeScenery::HandleCampaignStateChanged);
	}
	ShowNode(GI ? GI->CurrentNode : FallbackNode);
}

void AAetheriaNodeScenery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GI)
	{
		GI->OnCampaignStateChanged.RemoveDynamic(this, &AAetheriaNodeScenery::HandleCampaignStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void AAetheriaNodeScenery::HandleCampaignStateChanged()
{
	if (GI && GI->CurrentNode != ShownNode)
	{
		ShowNode(GI->CurrentNode);
	}
}

void AAetheriaNodeScenery::ShowNode(FName Node)
{
	ShownNode = Node;
	const FName Wanted(*(NodeTagPrefix + Node.ToString()));

	int32 Shown = 0;
	int32 Hidden = 0;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		bool bNodeActor = false;
		bool bThisNode = false;
		for (const FName& Tag : It->Tags)
		{
			if (Tag.ToString().StartsWith(NodeTagPrefix))
			{
				bNodeActor = true;
				bThisNode |= Tag == Wanted;
			}
		}
		if (bNodeActor)
		{
			It->SetActorHiddenInGame(!bThisNode);
			bThisNode ? ++Shown : ++Hidden;
		}
	}
	ApplyNodeLight(Node);
	UE_LOG(LogAetheria, Log, TEXT("Node scenery: %s (%d shown, %d hidden)"), *Node.ToString(), Shown, Hidden);
}

void AAetheriaNodeScenery::ApplyNodeLight(FName Node)
{
	const FLinearColor* Color = NodeLightColors.Find(Node);
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(KeyLightTag))
		{
			continue;
		}
		TArray<ULightComponent*> Lights;
		It->GetComponents(Lights);
		for (ULightComponent* Light : Lights)
		{
			const FLinearColor& Base = BaseLightColors.FindOrAdd(Light, Light->GetLightColor());
			Light->SetLightColor(Color ? *Color : Base);
		}
	}
}
