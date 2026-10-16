// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaNodeScenery.generated.h"

class UAetheriaGameInstance;

/**
 *  Shows the set pieces of the fleet's current star node and hides every other node's (WORLD_BIBLE.md 3.2).
 *  Set-piece actors carry a tag Node_<NodeId> (e.g. Node_EmberRelay); untagged actors are left alone.
 *  Place one in each level that shows the outside: L_Command (through the bridge viewport) and the combat map.
 *  Refreshes when the campaign state changes, so the view outside changes as a leap lands.
 */
UCLASS(Blueprintable)
class AAetheriaNodeScenery : public AActor
{
	GENERATED_BODY()

public:

	AAetheriaNodeScenery();

	/** Node shown when there is no Aetheria game instance (editor previews) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scenery")
	FName FallbackNode = TEXT("ArchWake");

	/** Shows the given node's set pieces and hides the others */
	UFUNCTION(BlueprintCallable, Category="Scenery")
	void ShowNode(FName Node);

	UFUNCTION(BlueprintPure, Category="Scenery")
	FName GetShownNode() const { return ShownNode; }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleCampaignStateChanged();

private:

	UPROPERTY(Transient)
	TObjectPtr<UAetheriaGameInstance> GI;

	FName ShownNode;
};
