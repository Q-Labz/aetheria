// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaTypes.h"
#include "AetheriaBridgeDirector.generated.h"

class ACameraActor;
class ULightComponent;
class UMaterialInstanceDynamic;

/**
 *  Camera direction on the Crystal Dreadnought's bridge (BRIDGE_SPEC.md B3). Placed in L_Command; cuts the
 *  command player's view between shot cameras tagged Shot_<Name>: the speaking officer during dialogue,
 *  the holotable for the star map and councils, the wide shot otherwise. When Vex speaks its light column
 *  brightens; when the Core Architect speaks the bridge lights dim (WORLD_BIBLE.md 5.3).
 *  Without a director in the level the command layer keeps its single CommandCamera.
 */
UCLASS(Blueprintable)
class AAetheriaBridgeDirector : public AActor
{
	GENERATED_BODY()

public:

	AAetheriaBridgeDirector();

	/** Shot camera tag per speaker; speakers without an entry use WideShot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shots")
	TMap<E_Speaker, FName> SpeakerShots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shots")
	FName WideShot = TEXT("Shot_Wide");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shots")
	FName CouncilShot = TEXT("Shot_Council");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shots")
	FName ChartShot = TEXT("Shot_Chart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shots")
	float BlendTime = 0.6f;

	/** Actor tag of Vex's light column */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bridge")
	FName VexColumnTag = TEXT("Bridge_VexColumn");

	/** Emissive Glow of the column at rest and while Vex speaks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bridge")
	FVector2D VexGlow = FVector2D(4.0f, 14.0f);

	/** Bridge light scale while the Core Architect speaks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bridge")
	float ArchitectDim = 0.9f;

	/** Cuts to the speaker's shot and sets the bridge's mood for them */
	UFUNCTION(BlueprintCallable, Category="Shots")
	void ShowSpeaker(E_Speaker Speaker);

	/** Cuts to the shot camera tagged Shot (falls back to WideShot) */
	UFUNCTION(BlueprintCallable, Category="Shots")
	void ShowShot(FName Shot);

	/** The first director in the world, or null */
	UFUNCTION(BlueprintPure, Category="Shots", meta=(WorldContext="WorldContextObject"))
	static AAetheriaBridgeDirector* Find(const UObject* WorldContextObject);

protected:

	virtual void BeginPlay() override;

private:

	void InitializeBridge();
	ACameraActor* FindShot(FName Shot);
	void SetVexSpeaking(bool bSpeaking);
	void SetArchitectSpeaking(bool bSpeaking);

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<ACameraActor>> Shots;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> VexMaterials;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ULightComponent>> BridgeLights;

	TArray<float> BaseIntensities;
	bool bArchitectDimmed = false;
	bool bInitialized = false;
};
