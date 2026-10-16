// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaNodeScenery.h"
#include "Aetheria.h"
#include "AetheriaGameInstance.h"
#include "EngineUtils.h"

namespace
{
	const FString NodeTagPrefix(TEXT("Node_"));
}

AAetheriaNodeScenery::AAetheriaNodeScenery()
{
	PrimaryActorTick.bCanEverTick = false;
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
	UE_LOG(LogAetheria, Log, TEXT("Node scenery: %s (%d shown, %d hidden)"), *Node.ToString(), Shown, Hidden);
}
