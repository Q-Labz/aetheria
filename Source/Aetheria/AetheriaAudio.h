// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UAudioComponent;
class USceneComponent;
class USoundAttenuation;
class USoundBase;
class USoundConcurrency;

/**
 *  Aetheria's sound set (BRIDGE_SPEC.md E): the synthesized waves in /Game/Aetheria/Audio, played by name.
 *  Built by Tools/Audio/make_sounds.py; A_* ambience loops, S_* effects, M_* music stings.
 *  Combat sounds are spatialized for the overhead camera (thousands of uu away) and capped per kind, so a full
 *  fleet firing never floods the mix. A missing sound is skipped silently.
 */
namespace AetheriaAudio
{
	/** How a positional sound carries */
	enum class ERange : uint8
	{
		Battle,		// weapons, explosions, pulses: heard from the overhead camera
		Engine,		// ship drives: only the nearer ships
		Bridge,		// console chirps on the bridge set
	};

	/** The sound /Game/Aetheria/Audio/<Name>, or null */
	USoundBase* Get(const FString& Name);

	/** One of Prefix_01 .. Prefix_<Count>, at random */
	FString Variant(const FString& Prefix, int32 Count);

	/** A non-positional sound that also plays while the game is paused (stings, UI) */
	void PlayUI(const UObject* WorldContext, const FString& Name, float Volume = 1.0f);

	/** A positional one-shot; Group limits how many of that kind play at once (empty: no limit) */
	void PlayAt(const UObject* WorldContext, const FString& Name, const FVector& Location, ERange Range,
		float Volume = 1.0f, float Pitch = 1.0f, FName Group = NAME_None);

	/** A looping sound attached to a component (engines), or a 2D bed when AttachTo is null */
	UAudioComponent* Loop(const UObject* WorldContext, const FString& Name, USceneComponent* AttachTo, ERange Range,
		float Volume = 1.0f, float Pitch = 1.0f);
}
