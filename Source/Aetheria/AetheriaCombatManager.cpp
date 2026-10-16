// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaCombatManager.h"
#include "AetheriaAfterAction.h"
#include "AetheriaFlagship.h"
#include "AetheriaGameInstance.h"
#include "AetheriaPhantomSquadron.h"
#include "AetheriaShipBase.h"
#include "AetheriaTacticalController.h"
#include "Aetheria.h"
#include "Engine/DataTable.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace AetheriaCombat
{
	const FName PlayerSpawnFlagship(TEXT("PlayerSpawn_Flagship"));
	const FName PlayerSpawnSq1(TEXT("PlayerSpawn_Sq1"));
	const FName PlayerSpawnSq2(TEXT("PlayerSpawn_Sq2"));
	const FName PlayerSpawnCorsair(TEXT("PlayerSpawn_Corsair"));
	const FName EnemySpawnCruiser(TEXT("EnemySpawn_Cruiser"));
	const FName EnemySpawn(TEXT("EnemySpawn"));

	constexpr float SurgeCost = 30.0f;
	constexpr float SurgeDuration = 6.0f;
	constexpr float SurgeDilation = 2.0f;
}

AAetheriaCombatManager::AAetheriaCombatManager()
{
	PrimaryActorTick.bCanEverTick = false;

	FlagshipClass = TSoftClassPtr<AAetheriaFlagship>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_Flagship.BP_Flagship_C")));
	SquadronClass = TSoftClassPtr<AAetheriaPhantomSquadron>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_PhantomSquadron.BP_PhantomSquadron_C")));
	CorsairClass = TSoftClassPtr<AAetheriaShipBase>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_CorsairGunship.BP_CorsairGunship_C")));
	CruiserClass = TSoftClassPtr<AAetheriaShipBase>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_TitheCruiser.BP_TitheCruiser_C")));
	InterceptorClass = TSoftClassPtr<AAetheriaShipBase>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_TitheInterceptor.BP_TitheInterceptor_C")));
	AfterActionClass = TSoftClassPtr<UAetheriaAfterAction>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_AfterAction.WBP_AfterAction_C")));
	CommandLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Aetheria/Maps/L_Command.L_Command")));
}

void AAetheriaCombatManager::BeginPlay()
{
	Super::BeginPlay();

	using namespace AetheriaCombat;

	UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	AAetheriaTacticalController* PC = GetTacticalController();
	if (PC)
	{
		PC->Manager = this;
	}

	auto FirstPoint = [this](FName Tag) -> FTransform
	{
		const TArray<AActor*> Points = FindSpawnPoints(Tag);
		return Points.Num() > 0 ? Points[0]->GetActorTransform() : GetActorTransform();
	};

	// ---- Player fleet ----

	if (UClass* Class = FlagshipClass.LoadSynchronous())
	{
		Flagship = Cast<AAetheriaFlagship>(SpawnShip(Class, FirstPoint(PlayerSpawnFlagship), FText::GetEmpty()));
	}

	if (UClass* Class = SquadronClass.LoadSynchronous())
	{
		const TPair<FName, const TCHAR*> SquadronSpawns[] = {
			{ PlayerSpawnSq1, TEXT("Phantom Squadron I") },
			{ PlayerSpawnSq2, TEXT("Phantom Squadron II") }
		};
		for (const TPair<FName, const TCHAR*>& Spawn : SquadronSpawns)
		{
			if (AAetheriaPhantomSquadron* Squadron = Cast<AAetheriaPhantomSquadron>(
				SpawnShip(Class, FirstPoint(Spawn.Key), FText::FromString(Spawn.Value))))
			{
				Squadrons.Add(Squadron);
			}
		}
	}

	if (GI && GI->Background == E_Background::OutcastCorsair)
	{
		if (UClass* Class = CorsairClass.LoadSynchronous())
		{
			SpawnShip(Class, FirstPoint(PlayerSpawnCorsair), FText::GetEmpty());
		}
	}

	for (AAetheriaShipBase* Ship : PlayerShips)
	{
		Ship->OnShipDestroyed.AddDynamic(this, &AAetheriaCombatManager::OnPlayerLost);
	}

	if (PC)
	{
		PC->Flagship = Flagship;
		PC->Squadron1 = Squadrons.IsValidIndex(0) ? Squadrons[0].Get() : nullptr;
		PC->Squadron2 = Squadrons.IsValidIndex(1) ? Squadrons[1].Get() : nullptr;
	}

	// ---- Enemies: a DT_Battles row (Act VII), or Vance's picket ----

	TArray<AAetheriaShipBase*> Enemies;
	const bool bTableBattle = SpawnBattle(GI, Enemies);
	if (!bTableBattle)
	{
	if (UClass* Class = CruiserClass.LoadSynchronous())
	{
		Enemies.Add(SpawnShip(Class, FirstPoint(EnemySpawnCruiser), FText::GetEmpty()));
	}

	if (UClass* Class = InterceptorClass.LoadSynchronous())
	{
		const int32 Count = 4 + ((GI && GI->PursuitClock >= 2) ? 2 : 0);
		const TArray<AActor*> Points = FindSpawnPoints(EnemySpawn);
		for (int32 Index = 0; Index < Count && Index < Points.Num(); ++Index)
		{
			Enemies.Add(SpawnShip(Class, Points[Index]->GetActorTransform(), FText::GetEmpty()));
		}
	}
	}

	for (AAetheriaShipBase* Enemy : Enemies)
	{
		if (Enemy)
		{
			Enemy->OnShipDestroyed.AddDynamic(this, &AAetheriaCombatManager::OnEnemyLost);
			EnemyShips.Add(Enemy);
			++AliveEnemies;
		}
	}

	// ---- Retry snapshot ----

	if (GI)
	{
		CachedInstability = GI->Instability;
		CachedFlagshipHullPct = GI->FlagshipHullPct;
		CachedPhantomShipsActive = GI->PhantomShipsActive;
	}

	if (bTableBattle)
	{
		for (const FS_Line& Line : Battle.OpeningComms)
		{
			Log(Line.Speaker, Line.Text.ToString());
		}
		if (Battle.HoldSeconds > 0.0f)
		{
			HoldRemaining = Battle.HoldSeconds;
			Log(E_Speaker::VexAI, FString::Printf(TEXT("Hold position for %d seconds, Captain."), FMath::RoundToInt(HoldRemaining)));
		}
		GetWorldTimerManager().SetTimer(BattleTimer, this, &AAetheriaCombatManager::BattleTick, 1.0f, true);
	}
	else
	{
		Log(E_Speaker::VexAI, TEXT("Contact. Sector 7 transponder codes. Commander Vance's picket, Captain."));
		Log(E_Speaker::CommanderVance, TEXT("Key-Bearer. You opened a door that should have stayed shut. I followed you through it."));
	}
	Log(E_Speaker::VexAI, TEXT("Tactical pause engaged. The fleet awaits your orders."));

	// the battle opens paused: orders first
	GetWorldTimerManager().SetTimer(OpenTimer, this, &AAetheriaCombatManager::OpenBattle, 0.2f, false);
}

void AAetheriaCombatManager::OpenBattle()
{
	FlushPendingLog();

	if (AAetheriaTacticalController* PC = GetTacticalController())
	{
		PC->SetTacPause(true);
	}
}

AAetheriaShipBase* AAetheriaCombatManager::SpawnShip(UClass* Class, const FTransform& Where, const FText& Name)
{
	if (!Class)
	{
		return nullptr;
	}

	// deferred so OnShipMessage is bound before the ship's BeginPlay speaks
	AAetheriaShipBase* Ship = GetWorld()->SpawnActorDeferred<AAetheriaShipBase>(
		Class, FTransform(Where.Rotator(), Where.GetLocation()), this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Ship)
	{
		return nullptr;
	}

	if (!Name.IsEmpty())
	{
		Ship->DisplayName = Name;
	}
	Ship->Manager = this;
	Ship->OnShipMessage.AddDynamic(this, &AAetheriaCombatManager::OnShipMessage);
	UGameplayStatics::FinishSpawningActor(Ship, FTransform(Where.Rotator(), Where.GetLocation()));

	if (Ship->Faction == E_Faction::Player)
	{
		PlayerShips.Add(Ship);
	}
	return Ship;
}

TArray<AActor*> AAetheriaCombatManager::FindSpawnPoints(FName Tag) const
{
	TArray<AActor*> Tagged;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ATargetPoint::StaticClass(), Tag, Tagged);
	return Tagged;
}

void AAetheriaCombatManager::OnShipMessage(AAetheriaShipBase* Ship, E_Speaker Speaker, const FText& Message)
{
	Log(Speaker, Message.ToString());
}

void AAetheriaCombatManager::Log(E_Speaker Speaker, const FString& Text)
{
	UE_LOG(LogAetheria, Log, TEXT("Comms [%s] %s"), *UEnum::GetValueAsString(Speaker), *Text);

	// lines raised before the controller has built its HUD wait for it
	AAetheriaTacticalController* PC = GetTacticalController();
	if (PC && PC->HUD)
	{
		FlushPendingLog();
		PC->Log(Speaker, Text);
	}
	else
	{
		PendingLog.Emplace(Speaker, Text);
	}
}

void AAetheriaCombatManager::FlushPendingLog()
{
	AAetheriaTacticalController* PC = GetTacticalController();
	if (!PC || !PC->HUD)
	{
		return;
	}

	for (const TPair<E_Speaker, FString>& Line : PendingLog)
	{
		PC->Log(Line.Key, Line.Value);
	}
	PendingLog.Reset();
}

void AAetheriaCombatManager::OnEnemyLost(AAetheriaShipBase* Ship)
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
	if (Ship && Ship->Tags.Contains(TEXT("BattleObjective")))
	{
		AliveObjectives = FMath::Max(0, AliveObjectives - 1);
		if (AliveObjectives == 0 && Battle.HoldSeconds <= 0.0f)
		{
			Victory();
			return;
		}
	}
	// defend battles end on the clock, not on kills
	if (AliveEnemies == 0 && Battle.HoldSeconds <= 0.0f)
	{
		Victory();
	}
}

bool AAetheriaCombatManager::SpawnBattle(UAetheriaGameInstance* GI, TArray<AAetheriaShipBase*>& OutEnemies)
{
	if (!GI || GI->PendingBattle.IsNone())
	{
		return false;
	}
	const UDataTable* Table = GI->BattlesTable.LoadSynchronous();
	const FS_Battle* Row = Table ? Table->FindRow<FS_Battle>(GI->PendingBattle, TEXT("SpawnBattle"), false) : nullptr;
	if (!Row || Row->Groups.Num() == 0)
	{
		UE_LOG(LogAetheria, Warning, TEXT("Battle %s is not in DT_Battles; staging the default engagement"), *GI->PendingBattle.ToString());
		return false;
	}
	BattleId = GI->PendingBattle;
	Battle = *Row;

	using namespace AetheriaCombat;
	TArray<AActor*> EnemyPoints = FindSpawnPoints(EnemySpawn);
	EnemyPoints.Append(FindSpawnPoints(EnemySpawnCruiser));
	const TArray<AActor*> AllyPoints = FindSpawnPoints(PlayerSpawnSq1);
	const FTransform AllyBase = AllyPoints.Num() > 0 ? AllyPoints[0]->GetActorTransform() : GetActorTransform();

	int32 EnemyIndex = 0;
	int32 AllyIndex = 0;
	for (const FS_BattleGroup& Group : Battle.Groups)
	{
		UClass* Class = Group.ShipClass.LoadSynchronous();
		if (!Class)
		{
			UE_LOG(LogAetheria, Warning, TEXT("Battle %s: a group has no ship class"), *BattleId.ToString());
			continue;
		}
		for (int32 i = 0; i < Group.Count; ++i)
		{
			FTransform Where;
			if (Group.bAllied)
			{
				Where = AllyBase;
				Where.AddToTranslation(FVector(-600.0f * (AllyIndex / 3), (AllyIndex % 3 - 1) * 900.0f, 0.0f));
				++AllyIndex;
			}
			else if (EnemyPoints.Num() > 0)
			{
				// cycle the enemy points, pushing later rounds further out
				Where = EnemyPoints[EnemyIndex % EnemyPoints.Num()]->GetActorTransform();
				Where.AddToTranslation(FVector(700.0f * (EnemyIndex / EnemyPoints.Num()), 0.0f, 0.0f));
				++EnemyIndex;
			}
			else
			{
				Where = GetActorTransform();
			}

			AAetheriaShipBase* Ship = SpawnShip(Class, Where, FText::GetEmpty());
			if (!Ship)
			{
				continue;
			}
			if (Group.bAllied)
			{
				Ship->Faction = E_Faction::Player;
				Ship->bTargetsFlagship = false;
				PlayerShips.AddUnique(Ship);
				Ship->OnShipDestroyed.AddDynamic(this, &AAetheriaCombatManager::OnPlayerLost);
				continue;
			}
			if (Group.bObjective)
			{
				Ship->Tags.Add(TEXT("BattleObjective"));
				++AliveObjectives;
			}
			OutEnemies.Add(Ship);
		}
	}
	return OutEnemies.Num() > 0;
}

void AAetheriaCombatManager::BattleTick()
{
	if (bBattleOver)
	{
		GetWorldTimerManager().ClearTimer(BattleTimer);
		return;
	}

	if (Battle.HoldSeconds > 0.0f)
	{
		HoldRemaining -= 1.0f;
		const int32 Left = FMath::RoundToInt(HoldRemaining);
		if (Left == 60 || Left == 30 || Left == 10)
		{
			Log(E_Speaker::VexAI, FString::Printf(TEXT("%d seconds, Captain."), Left));
		}
		if (HoldRemaining <= 0.0f)
		{
			Log(E_Speaker::VexAI, TEXT("The line held."));
			Victory();
			return;
		}
	}

	if (Battle.WithdrawAtHullPct > 0.0f)
	{
		float Hull = 0.0f;
		float MaxHull = 0.0f;
		for (const AAetheriaShipBase* Enemy : EnemyShips)
		{
			if (Enemy)
			{
				Hull += Enemy->IsAlive() ? Enemy->Hull : 0.0f;
				MaxHull += Enemy->MaxHull;
			}
		}
		if (MaxHull > 0.0f && Hull / MaxHull <= Battle.WithdrawAtHullPct)
		{
			Log(E_Speaker::VexAI, TEXT("The enemy is withdrawing."));
			Victory();
		}
	}
}

void AAetheriaCombatManager::OnPlayerLost(AAetheriaShipBase* Ship)
{
	if (Ship == Flagship)
	{
		Defeat();
		return;
	}

	if (Cast<AAetheriaPhantomSquadron>(Ship))
	{
		Log(E_Speaker::VexAI, FString::Printf(TEXT("%s lost. Twenty vessels of the Phantom Fleet."), *Ship->DisplayName.ToString()));
	}
	else if (Ship)
	{
		Log(E_Speaker::VexAI, FString::Printf(TEXT("%s lost."), *Ship->DisplayName.ToString()));
	}
}

void AAetheriaCombatManager::StartSurge()
{
	UAetheriaGameInstance* GI = GetAetheriaGameInstance();
	if (bSurgeActive || bBattleOver || !GI || GI->ChronoCharge < AetheriaCombat::SurgeCost)
	{
		return;
	}

	bSurgeActive = true;
	GI->ChronoCharge -= AetheriaCombat::SurgeCost;

	SurgedShips.Reset();
	for (AAetheriaShipBase* Ship : PlayerShips)
	{
		if (Ship && Ship->IsAlive())
		{
			Ship->CustomTimeDilation = AetheriaCombat::SurgeDilation;
			Ship->DamageMultiplier *= 2.0f;
			SurgedShips.Add(Ship);
		}
	}

	Log(E_Speaker::JaxMercer, TEXT("Chrono-Core surge. Fleet temporal acceleration for six seconds."));

	// on the manager, which keeps normal time
	GetWorldTimerManager().SetTimer(SurgeTimer, this, &AAetheriaCombatManager::EndSurge, AetheriaCombat::SurgeDuration, false);
}

void AAetheriaCombatManager::EndSurge()
{
	for (AAetheriaShipBase* Ship : SurgedShips)
	{
		if (IsValid(Ship))
		{
			Ship->CustomTimeDilation = 1.0f;
			Ship->DamageMultiplier /= 2.0f;
		}
	}
	SurgedShips.Reset();
	bSurgeActive = false;
}

int32 AAetheriaCombatManager::GetPhantomLosses() const
{
	int32 Losses = 0;
	for (const AAetheriaPhantomSquadron* Squadron : Squadrons)
	{
		if (Squadron)
		{
			Losses += Squadron->ShipsInSquadron - (Squadron->IsAlive() ? Squadron->GetAliveShips() : 0);
		}
	}
	return Losses;
}

int32 AAetheriaCombatManager::GetPhantomFleetStrength() const
{
	const int32 Base = bBattleOver ? CachedPhantomShipsActive : (GetAetheriaGameInstance() ? GetAetheriaGameInstance()->PhantomShipsActive : 200);
	return FMath::Max(0, Base - GetPhantomLosses());
}

void AAetheriaCombatManager::Victory()
{
	if (bBattleOver)
	{
		return;
	}
	bBattleOver = true;
	EndSurge();

	const int32 Losses = GetPhantomLosses();
	const float HullPct = (Flagship && Flagship->MaxHull > 0.0f) ? Flagship->Hull / Flagship->MaxHull : 0.0f;

	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->FlagshipHullPct = HullPct;
		GI->PhantomShipsActive -= Losses;
		if (BattleId.IsNone())
		{
			GI->StoryFlags.Add(TEXT("LanternCleared"));
		}
		else
		{
			GI->StoryFlags.Add(FName(*(TEXT("Won_") + BattleId.ToString())));
			for (const FName& Flag : Battle.WinFlags)
			{
				GI->StoryFlags.Add(Flag);
			}
		}
		GI->NextScreen = TEXT("PostCombat");
	}

	const int32 FleetStrength = GetAetheriaGameInstance() ? GetAetheriaGameInstance()->PhantomShipsActive : 0;
	const FString Body = FString::Printf(
		TEXT("Result: Hostile picket destroyed.\nCrystal Dreadnought hull integrity: %d%%.\nPhantom Fleet losses: %d vessels. Fleet strength: %d of 200.\nPrismatic Stellar Beam discharges: %d. Hostiles destroyed by beam: %d.\nReport ends."),
		FMath::RoundToInt(HullPct * 100.0f), Losses, FleetStrength,
		Flagship ? Flagship->BeamShots : 0, Flagship ? Flagship->BeamKills : 0);

	const FText Title = BattleId.IsNone() || Battle.Title.IsEmpty()
		? FText::FromString(TEXT("MISSION REPORT — ENGAGEMENT AT THE LANTERN DRIFT"))
		: FText::FromString(TEXT("MISSION REPORT — ") + Battle.Title.ToString().ToUpper());
	ShowAfterAction(true, Title, FText::FromString(Body));

	if (AAetheriaTacticalController* PC = GetTacticalController())
	{
		PC->SetTacPause(true);
	}
}

void AAetheriaCombatManager::Defeat()
{
	if (bBattleOver)
	{
		return;
	}
	bBattleOver = true;
	EndSurge();

	// the engagement is undone: Retry reopens it, Return to Command withdraws to the star map, not the victory debrief
	RestoreCachedCampaign();
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->NextScreen = TEXT("StarMap");
	}

	ShowAfterAction(false, FText::FromString(TEXT("MISSION REPORT")),
		FText::FromString(TEXT("The Crystal Dreadnought has been lost. The Key-Bearer's flag is struck. The armada scatters into the dark.")));

	if (AAetheriaTacticalController* PC = GetTacticalController())
	{
		PC->SetTacPause(true);
	}
}

void AAetheriaCombatManager::ShowAfterAction(bool bWon, const FText& Title, const FText& Body)
{
	AAetheriaTacticalController* PC = GetTacticalController();
	UClass* Class = AfterActionClass.LoadSynchronous();
	if (!PC || !Class)
	{
		UE_LOG(LogAetheria, Warning, TEXT("After-action report could not be shown: %s"), *Body.ToString());
		return;
	}

	if (UAetheriaAfterAction* Report = CreateWidget<UAetheriaAfterAction>(PC, Class))
	{
		Report->Setup(this, bWon, Title, Body);
		Report->AddToViewport(10);
	}
}

void AAetheriaCombatManager::ReturnToCommand()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, CommandLevel);
}

void AAetheriaCombatManager::RetryEngagement()
{
	RestoreCachedCampaign();

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AAetheriaCombatManager::RestoreCachedCampaign()
{
	if (UAetheriaGameInstance* GI = GetAetheriaGameInstance())
	{
		GI->Instability = CachedInstability;
		GI->FlagshipHullPct = CachedFlagshipHullPct;
		GI->PhantomShipsActive = CachedPhantomShipsActive;
	}
}

UAetheriaGameInstance* AAetheriaCombatManager::GetAetheriaGameInstance() const
{
	return Cast<UAetheriaGameInstance>(GetGameInstance());
}

AAetheriaTacticalController* AAetheriaCombatManager::GetTacticalController() const
{
	return Cast<AAetheriaTacticalController>(UGameplayStatics::GetPlayerController(this, 0));
}
