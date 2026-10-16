// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaNodeScenery.generated.h"

class UAetheriaGameInstance;
class ULightComponent;

/**
 *  Shows the set pieces of the fleet's current star node and hides every other node's (WORLD_BIBLE.md 3.2).
 *  Set-piece actors carry a tag Node_<NodeId> (e.g. Node_EmberRelay); untagged actors are left alone.
 *  Place one in each level that shows the outside: L_Command (through the bridge viewport) and the combat map.
 *  Refreshes when the campaign state changes, so the view outside changes as a leap lands.
 *  Also sets the node's light: lights tagged KeyLightTag take the colour of the node's star (NodeLightColors).
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

	/** Actor tag of the key lights that take the node's star colour (the sun through the viewport, the combat key) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	FName KeyLightTag = TEXT("NodeKeyLight");

	/** Key light colour per node; nodes without an entry keep the light's own colour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	TMap<FName, FLinearColor> NodeLightColors;

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

	void ApplyNodeLight(FName Node);

	UPROPERTY(Transient)
	TObjectPtr<UAetheriaGameInstance> GI;

	/** Each key light's colour as placed, restored for nodes without a colour */
	UPROPERTY(Transient)
	TMap<TObjectPtr<ULightComponent>, FLinearColor> BaseLightColors;

	FName ShownNode;
};
