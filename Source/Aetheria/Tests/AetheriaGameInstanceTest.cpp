// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AetheriaGameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace AetheriaGameInstanceTest
{
	struct FLeap
	{
		float Cost = 0.0f;
		float Projected = 0.0f;
		bool bWillOverflow = false;
		bool bFound = false;
	};

	FLeap LeapCost(const UAetheriaGameInstance* GI, FName To)
	{
		FLeap Leap;
		GI->GetLeapCost(To, Leap.Cost, Leap.Projected, Leap.bWillOverflow, Leap.bFound);
		return Leap;
	}
}

/** CLAUDE.md phase 3 check and the leap design math in SLICE_SPEC.md section 3 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetheriaGameInstanceLeapCostTest, "Aetheria.GameInstance.LeapCost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetheriaGameInstanceLeapCostTest::RunTest(const FString& Parameters)
{
	using namespace AetheriaGameInstanceTest;

	// Direct leap from the Arch Wake at defaults: 25 + 80 = 105, overflow
	{
		UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();
		const FLeap Leap = LeapCost(GI, TEXT("EmberRelay"));
		TestTrue(TEXT("Direct: route found"), Leap.bFound);
		TestEqual(TEXT("Direct: cost"), Leap.Cost, 80.0f);
		TestEqual(TEXT("Direct: projected"), Leap.Projected, 105.0f);
		TestTrue(TEXT("Direct: overflows"), Leap.bWillOverflow);

		TestFalse(TEXT("Unknown lane is not found"), LeapCost(GI, TEXT("Nowhere")).bFound);
	}

	// Two-step: 25 + 35 = 60 (Strained), then 30 x 1.25 = 37.5 -> 97.5 Critical
	{
		UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();
		const FLeap First = LeapCost(GI, TEXT("LanternDrift"));
		TestEqual(TEXT("Two-step: first projected"), First.Projected, 60.0f);
		TestFalse(TEXT("Two-step: first holds"), First.bWillOverflow);

		const E_Encounter Encounter = GI->ExecuteLeap(TEXT("LanternDrift"));
		TestTrue(TEXT("Lantern Drift is a combat encounter"), Encounter == E_Encounter::Combat);
		TestEqual(TEXT("Two-step: instability after first leap"), GI->Instability, 60.0f);
		TestEqual(TEXT("Two-step: now at Lantern Drift"), GI->CurrentNode, FName(TEXT("LanternDrift")));

		const FLeap Second = LeapCost(GI, TEXT("EmberRelay"));
		TestEqual(TEXT("Two-step: strained cost"), Second.Cost, 37.5f);
		TestEqual(TEXT("Two-step: second projected"), Second.Projected, 97.5f);
		TestFalse(TEXT("Two-step: second holds"), Second.bWillOverflow);
	}

	// Chart that lane -> 78.75
	{
		UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();
		GI->ExecuteLeap(TEXT("LanternDrift"));
		GI->ChartRoute(TEXT("EmberRelay"));
		TestEqual(TEXT("Charted: projected"), LeapCost(GI, TEXT("EmberRelay")).Projected, 78.75f);
		TestEqual(TEXT("Charted: pursuit clock advanced"), GI->PursuitClock, 1);
	}

	// Vent first -> 40 + 30 = 70
	{
		UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();
		GI->ExecuteLeap(TEXT("LanternDrift"));
		GI->VentChrono();
		TestEqual(TEXT("Vent: chrono spent"), GI->ChronoCharge, 35.0f);
		TestEqual(TEXT("Vent: projected"), LeapCost(GI, TEXT("EmberRelay")).Projected, 70.0f);
	}

	// Guild Engineers leap 10% cheaper
	{
		UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();
		GI->Background = E_Background::GuildEngineer;
		TestEqual(TEXT("Guild Engineer: cost"), LeapCost(GI, TEXT("EmberRelay")).Cost, 72.0f);
	}

	return true;
}

/** First rupture from the direct leap: excess 5, halved by the external harmonic */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetheriaGameInstanceRuptureTest, "Aetheria.GameInstance.Rupture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetheriaGameInstanceRuptureTest::RunTest(const FString& Parameters)
{
	UAetheriaGameInstance* GI = NewObject<UAetheriaGameInstance>();

	const E_Encounter Encounter = GI->ExecuteLeap(TEXT("EmberRelay"));
	TestTrue(TEXT("Ember Relay is a council encounter"), Encounter == E_Encounter::Council);
	TestTrue(TEXT("First rupture absorbed"), GI->FirstRuptureAbsorbed);
	TestTrue(TEXT("AnomalousDamping flag set"), GI->StoryFlags.Contains(TEXT("AnomalousDamping")));
	TestEqual(TEXT("Architect exposure"), GI->ArchitectExposure, 1);
	TestEqual(TEXT("Instability resets to 60"), GI->Instability, 60.0f);
	TestEqual(TEXT("Hull loss 0.08 + 2.5 x 0.012"), GI->FlagshipHullPct, 0.89f, 0.0001f);
	TestEqual(TEXT("One crew injured"), GI->CrewInjured, 1);
	TestEqual(TEXT("No Phantom vessels lost"), GI->PhantomShipsActive, 200);
	TestTrue(TEXT("Report mentions the external harmonic"), GI->LastReport.ToString().Contains(TEXT("external harmonic")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
