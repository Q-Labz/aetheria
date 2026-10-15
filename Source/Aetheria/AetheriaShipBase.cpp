// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaShipBase.h"
#include "AetheriaBeamVFX.h"
#include "AetheriaGameInstance.h"
#include "AetheriaShipBar.h"
#include "Aetheria.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace AetheriaShip
{
	const FName FlagshipTag(TEXT("Flagship"));
	const FLinearColor PlayerTracer(0.3f, 0.8f, 1.0f);
	const FLinearColor EnemyTracer(1.0f, 0.2f, 0.05f);
	const FLinearColor SelectionColor(0.0f, 1.0f, 1.0f);
}

AAetheriaShipBase::AAetheriaShipBase()
{
	// Tick stops while paused, like the combat timers; tactical pause relies on that
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(Root);
	Collision->InitSphereRadius(350.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HullMesh"));
	HullMesh->SetupAttachment(Collision);
	HullMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SelectionRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionRing"));
	SelectionRing->SetupAttachment(Collision);
	SelectionRing->SetRelativeScale3D(FVector(8.0f, 8.0f, 0.02f));
	SelectionRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionRing->SetCastShadow(false);
	SelectionRing->SetHiddenInGame(true);
	SelectionRing->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder.Succeeded())
	{
		SelectionRing->SetStaticMesh(Cylinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	if (Emissive.Succeeded())
	{
		SelectionRing->SetMaterial(0, Emissive.Object);
	}

	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(Collision);
	Muzzle->SetRelativeLocation(FVector(400.0f, 0.0f, 0.0f));

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(Root);
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBar->SetDrawSize(FVector2D(140.0f, 22.0f));
	HealthBar->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));
	HealthBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BeamVFXClass = TSoftClassPtr<AAetheriaBeamVFX>(FSoftObjectPath(TEXT("/Game/Aetheria/Ships/BP_BeamVFX.BP_BeamVFX_C")));
	ShipBarClass = TSoftClassPtr<UAetheriaShipBar>(FSoftObjectPath(TEXT("/Game/Aetheria/UI/WBP_ShipBar.WBP_ShipBar_C")));
}

void AAetheriaShipBase::BeginPlay()
{
	Super::BeginPlay();

	Hull = MaxHull;
	Shield = MaxShield;

	if (Faction == E_Faction::VanceRemnant && TracerColor.Equals(AetheriaShip::PlayerTracer))
	{
		TracerColor = AetheriaShip::EnemyTracer;
	}

	// these timers halt while the game is paused
	FTimerManager& Timers = GetWorldTimerManager();
	if (FireInterval > 0.0f)
	{
		Timers.SetTimer(FireTimer, this, &AAetheriaShipBase::FireTick, FireInterval, true);
	}
	Timers.SetTimer(RegenTimer, this, &AAetheriaShipBase::RegenTick, 0.25f, true);
	if (Faction != E_Faction::Player)
	{
		Timers.SetTimer(AITimer, this, &AAetheriaShipBase::AIThink, 1.0f, true);
	}

	if (UMaterialInstanceDynamic* RingMaterial = SelectionRing->CreateDynamicMaterialInstance(0))
	{
		RingMaterial->SetVectorParameterValue(TEXT("Color"), AetheriaShip::SelectionColor);
	}

	if (!HealthBar->GetWidgetClass())
	{
		if (UClass* BarClass = ShipBarClass.LoadSynchronous())
		{
			HealthBar->SetWidgetClass(BarClass);
		}
	}
	HealthBar->InitWidget();
	if (UAetheriaShipBar* Bar = Cast<UAetheriaShipBar>(HealthBar->GetUserWidgetObject()))
	{
		Bar->SetOwnerShip(this);
	}
}

void AAetheriaShipBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void AAetheriaShipBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead || !CanManeuver())
	{
		return;
	}

	if (AttackTarget && !AttackTarget->IsAlive())
	{
		AttackTarget = nullptr;
	}

	if (bHasMoveOrder)
	{
		if (SteerTowards(MoveTarget, DeltaSeconds, 60.0f))
		{
			bHasMoveOrder = false;
		}
	}
	else if (AttackTarget)
	{
		const FVector TargetLocation = AttackTarget->GetActorLocation();
		if (FVector::Dist2D(GetActorLocation(), TargetLocation) > WeaponRange * 0.85f)
		{
			SteerTowards(TargetLocation, DeltaSeconds, 0.0f);
		}
		else
		{
			FaceTowards(TargetLocation, DeltaSeconds);
		}
	}
}

bool AAetheriaShipBase::SteerTowards(const FVector& Destination, float DeltaSeconds, float Tolerance)
{
	const FVector Location = GetActorLocation();
	const FVector Flat(Destination.X, Destination.Y, Location.Z);
	if (FVector::Dist2D(Location, Flat) <= Tolerance)
	{
		return true;
	}

	FaceTowards(Flat, DeltaSeconds);
	const FVector NewLocation = FMath::VInterpConstantTo(Location, Flat, DeltaSeconds, MoveSpeed);
	SetActorLocation(NewLocation);
	return FVector::Dist2D(NewLocation, Flat) <= Tolerance;
}

void AAetheriaShipBase::FaceTowards(const FVector& Point, float DeltaSeconds)
{
	const FVector Direction = (Point - GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator Desired(0.0f, Direction.Rotation().Yaw, 0.0f);
	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), Desired, DeltaSeconds, TurnSpeed));
}

void AAetheriaShipBase::IssueMove(FVector Location)
{
	MoveTarget = FVector(Location.X, Location.Y, 0.0f);
	bHasMoveOrder = true;
	AttackTarget = nullptr;
}

void AAetheriaShipBase::IssueAttack(AAetheriaShipBase* Target)
{
	AttackTarget = Target;
	bHasMoveOrder = false;
}

void AAetheriaShipBase::SetSelected(bool bSelected)
{
	SelectionRing->SetHiddenInGame(!bSelected);
	SelectionRing->SetVisibility(bSelected);
}

bool AAetheriaShipBase::IsHostileTo(const AAetheriaShipBase* Other) const
{
	return Other && Other->Faction != Faction;
}

AAetheriaShipBase* AAetheriaShipBase::FindNearestHostile(float Range) const
{
	AAetheriaShipBase* Nearest = nullptr;
	float NearestDistSq = FMath::Square(Range);

	for (TActorIterator<AAetheriaShipBase> It(GetWorld()); It; ++It)
	{
		AAetheriaShipBase* Ship = *It;
		if (Ship == this || !Ship->IsAlive() || !IsHostileTo(Ship))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared2D(GetActorLocation(), Ship->GetActorLocation());
		if (DistSq <= NearestDistSq)
		{
			Nearest = Ship;
			NearestDistSq = DistSq;
		}
	}
	return Nearest;
}

AAetheriaShipBase* AAetheriaShipBase::FindFlagship(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AAetheriaShipBase> It(World); It; ++It)
	{
		if (It->IsAlive() && It->Tags.Contains(AetheriaShip::FlagshipTag))
		{
			return *It;
		}
	}
	return nullptr;
}

FText AAetheriaShipBase::GetBarLabel() const
{
	return DisplayName;
}

void AAetheriaShipBase::FireTick()
{
	if (bIsDead)
	{
		return;
	}

	AAetheriaShipBase* Target = nullptr;
	if (AttackTarget && AttackTarget->IsAlive()
		&& FVector::Dist2D(GetActorLocation(), AttackTarget->GetActorLocation()) <= WeaponRange)
	{
		Target = AttackTarget;
	}
	else
	{
		Target = FindNearestHostile(WeaponRange);
	}

	if (!Target)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, WeaponDamage * DamageMultiplier, nullptr, this, UDamageType::StaticClass());
	SpawnBeam(Muzzle->GetComponentLocation(), Target->GetActorLocation(), 25.0f, TracerColor, 0.12f);
}

float AAetheriaShipBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Amount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bIsDead || Amount <= 0.0f)
	{
		return 0.0f;
	}

	LastHitTime = GetGameTime();

	// shields absorb first, then hull
	const float Absorbed = FMath::Min(Shield, Amount);
	Shield -= Absorbed;
	Hull = FMath::Max(0.0f, Hull - (Amount - Absorbed));

	OnDamaged(Amount);
	OnShipDamaged.Broadcast(this);

	if (Hull <= 0.0f)
	{
		Die();
	}
	return Amount;
}

void AAetheriaShipBase::RegenTick()
{
	if (!bIsDead && GetGameTime() - LastHitTime >= ShieldRegenDelay)
	{
		Shield = FMath::Min(MaxShield, Shield + ShieldRegenPerSec * 0.25f);
	}
}

void AAetheriaShipBase::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	Hull = 0.0f;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	OnShipDestroyed.Broadcast(this);

	// brief flash where the ship was
	const FVector Location = GetActorLocation();
	SpawnBeam(Location - FVector(0.0f, 0.0f, 200.0f), Location + FVector(0.0f, 0.0f, 200.0f),
		Collision->GetScaledSphereRadius() * 1.5f, FLinearColor::White, 0.25f);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	SetLifeSpan(2.0f);
}

void AAetheriaShipBase::AIThink()
{
	if (bIsDead)
	{
		return;
	}

	AAetheriaShipBase* Flagship = FindFlagship(this);

	if (bTargetsFlagship && Flagship)
	{
		if (AttackTarget != Flagship)
		{
			IssueAttack(Flagship);
		}
		return;
	}

	if (AAetheriaShipBase* Nearest = FindNearestHostile(AggroRange))
	{
		if (AttackTarget != Nearest)
		{
			IssueAttack(Nearest);
		}
	}
	else if (Flagship)
	{
		IssueMove(Flagship->GetActorLocation());
	}
}

AAetheriaBeamVFX* AAetheriaShipBase::SpawnBeam(const FVector& Start, const FVector& End, float Width, const FLinearColor& Color, float Life) const
{
	UClass* Class = BeamVFXClass.LoadSynchronous();
	if (!Class)
	{
		Class = AAetheriaBeamVFX::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAetheriaBeamVFX* Beam = GetWorld()->SpawnActor<AAetheriaBeamVFX>(Class, FTransform(Start), Params);
	if (Beam)
	{
		Beam->Setup(Start, End, Width, Color, Life);
	}
	return Beam;
}

void AAetheriaShipBase::PostComms(E_Speaker Speaker, const FString& Message)
{
	UE_LOG(LogAetheria, Log, TEXT("[%s] %s"), *UEnum::GetValueAsString(Speaker), *Message);
	OnShipMessage.Broadcast(this, Speaker, FText::FromString(Message));
}

UAetheriaGameInstance* AAetheriaShipBase::GetAetheriaGameInstance() const
{
	return Cast<UAetheriaGameInstance>(GetGameInstance());
}

float AAetheriaShipBase::GetGameTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}
