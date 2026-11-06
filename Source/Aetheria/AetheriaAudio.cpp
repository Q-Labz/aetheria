// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaAudio.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundConcurrency.h"

namespace AetheriaAudio
{
	namespace
	{
		const TCHAR* AudioFolder = TEXT("/Game/Aetheria/Audio/");

		USoundAttenuation* MakeAttenuation(float Inner, float Falloff)
		{
			USoundAttenuation* Attenuation = NewObject<USoundAttenuation>(GetTransientPackage());
			Attenuation->AddToRoot();
			FSoundAttenuationSettings& Settings = Attenuation->Attenuation;
			Settings.bAttenuate = true;
			Settings.bSpatialize = true;
			Settings.AttenuationShape = EAttenuationShape::Sphere;
			Settings.AttenuationShapeExtents = FVector(Inner, 0.0f, 0.0f);
			Settings.FalloffDistance = Falloff;
			Settings.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
			Settings.StereoSpread = 400.0f;
			return Attenuation;
		}

		USoundAttenuation* AttenuationFor(ERange Range)
		{
			// the overhead camera sits 2,500 - 14,000 uu from the battle plane
			static USoundAttenuation* Battle = MakeAttenuation(9000.0f, 16000.0f);
			static USoundAttenuation* Engine = MakeAttenuation(7000.0f, 9000.0f);
			// the bridge shots sit up to 2,000 uu from the consoles
			static USoundAttenuation* Bridge = MakeAttenuation(1200.0f, 3000.0f);
			switch (Range)
			{
			case ERange::Engine: return Engine;
			case ERange::Bridge: return Bridge;
			default: return Battle;
			}
		}

		USoundConcurrency* ConcurrencyFor(FName Group)
		{
			static TMap<FName, USoundConcurrency*> Groups;
			if (Group.IsNone())
			{
				return nullptr;
			}
			if (USoundConcurrency** Found = Groups.Find(Group))
			{
				return *Found;
			}
			USoundConcurrency* Concurrency = NewObject<USoundConcurrency>(GetTransientPackage());
			Concurrency->AddToRoot();
			Concurrency->Concurrency.MaxCount = 6;
			Concurrency->Concurrency.ResolutionRule = EMaxConcurrentResolutionRule::StopOldest;
			Concurrency->Concurrency.bLimitToOwner = false;
			Groups.Add(Group, Concurrency);
			return Concurrency;
		}
	}

	USoundBase* Get(const FString& Name)
	{
		const FString Path = FString(AudioFolder) + Name + TEXT(".") + Name;
		return LoadObject<USoundBase>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	}

	FString Variant(const FString& Prefix, int32 Count)
	{
		return FString::Printf(TEXT("%s_%02d"), *Prefix, FMath::RandRange(1, FMath::Max(1, Count)));
	}

	void PlayUI(const UObject* WorldContext, const FString& Name, float Volume)
	{
		if (USoundBase* Sound = Get(Name))
		{
			UGameplayStatics::PlaySound2D(WorldContext, Sound, Volume, 1.0f, 0.0f, nullptr, nullptr, true);
		}
	}

	void PlayAt(const UObject* WorldContext, const FString& Name, const FVector& Location, ERange Range,
		float Volume, float Pitch, FName Group)
	{
		if (USoundBase* Sound = Get(Name))
		{
			UGameplayStatics::PlaySoundAtLocation(WorldContext, Sound, Location, FRotator::ZeroRotator, Volume, Pitch,
				0.0f, AttenuationFor(Range), ConcurrencyFor(Group));
		}
	}

	UAudioComponent* Loop(const UObject* WorldContext, const FString& Name, USceneComponent* AttachTo, ERange Range,
		float Volume, float Pitch)
	{
		USoundBase* Sound = Get(Name);
		if (!Sound)
		{
			return nullptr;
		}
		if (AttachTo)
		{
			return UGameplayStatics::SpawnSoundAttached(Sound, AttachTo, NAME_None, FVector::ZeroVector,
				EAttachLocation::KeepRelativeOffset, true, Volume, Pitch, 0.0f, AttenuationFor(Range));
		}
		return UGameplayStatics::SpawnSound2D(WorldContext, Sound, Volume, Pitch);
	}
}
