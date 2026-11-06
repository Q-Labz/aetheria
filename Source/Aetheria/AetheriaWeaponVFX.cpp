// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaWeaponVFX.h"
#include "AetheriaAudio.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace AetheriaWeaponVFX
{
	void SetupMesh(UStaticMeshComponent* Mesh, UStaticMesh* Shape, UMaterialInterface* Material)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		Mesh->SetStaticMesh(Shape);
		Mesh->SetMaterial(0, Material);
	}

	FActorSpawnParameters SpawnParams()
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return Params;
	}
}

// ---- AAetheriaImpactFlash ------------------------------------------------------------------------

AAetheriaImpactFlash::AAetheriaImpactFlash()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LaserGlow(TEXT("/Game/Aetheria/Materials/M_LaserGlow.M_LaserGlow"));

	Halo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Halo"));
	RootComponent = Halo;
	AetheriaWeaponVFX::SetupMesh(Halo, Sphere.Object, LaserGlow.Object);

	Core = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Core"));
	Core->SetupAttachment(Halo);
	Core->SetRelativeScale3D(FVector(0.35f));
	AetheriaWeaponVFX::SetupMesh(Core, Sphere.Object, Emissive.Object);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Halo);
	Light->SetCastShadows(false);
	Light->SetIntensityUnits(ELightUnits::Candelas);
}

void AAetheriaImpactFlash::Setup(const FLinearColor& Color, float Size, float Life, float LightIntensity, bool bInGrow)
{
	FullSize = Size;
	TotalLife = FMath::Max(Life, 0.05f);
	BaseLight = LightIntensity;
	bGrow = bInGrow;
	Age = 0.0f;

	CoreMaterial = Core->CreateDynamicMaterialInstance(0);
	HaloMaterial = Halo->CreateDynamicMaterialInstance(0);
	if (CoreMaterial)
	{
		CoreMaterial->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(Color, FLinearColor::White, 0.6f));
		CoreMaterial->SetScalarParameterValue(TEXT("Glow"), 30.0f);
	}
	if (HaloMaterial)
	{
		HaloMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		// a soft halo: dim and strongly edge-faded, so the white core reads as the hot centre
		HaloMaterial->SetScalarParameterValue(TEXT("Glow"), 3.0f);
		HaloMaterial->SetScalarParameterValue(TEXT("Softness"), 3.5f);
	}

	Light->SetLightColor(Color);
	Light->SetAttenuationRadius(Size * 6.0f);
	SetLifeSpan(TotalLife);
	Apply(0.0f);
}

AAetheriaImpactFlash* AAetheriaImpactFlash::Spawn(UWorld* World, const FVector& Location, const FLinearColor& Color,
	float Size, float Life, float LightIntensity, bool bGrow)
{
	if (!World)
	{
		return nullptr;
	}
	AAetheriaImpactFlash* Flash = World->SpawnActor<AAetheriaImpactFlash>(AAetheriaImpactFlash::StaticClass(),
		FTransform(Location), AetheriaWeaponVFX::SpawnParams());
	if (Flash)
	{
		Flash->Setup(Color, Size, Life, LightIntensity, bGrow);
	}
	return Flash;
}

void AAetheriaImpactFlash::SetHaloOnly(float Glow)
{
	bHaloOnly = true;
	Core->SetVisibility(false);
	if (HaloMaterial)
	{
		HaloMaterial->SetScalarParameterValue(TEXT("Glow"), Glow);
	}
}

void AAetheriaImpactFlash::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	Apply(FMath::Clamp(Age / TotalLife, 0.0f, 1.0f));
}

void AAetheriaImpactFlash::Apply(float Alpha)
{
	// a burst flares fast then fades while expanding; a charge-up swells and brightens
	const float Scale = bGrow ? FMath::Lerp(0.15f, 1.0f, FMath::Sqrt(Alpha)) : FMath::Lerp(0.5f, 1.0f, FMath::Sqrt(Alpha));
	const float Brightness = bGrow ? FMath::Lerp(0.2f, 1.0f, Alpha) : FMath::Square(1.0f - Alpha);

	SetActorScale3D(FVector(FullSize / 100.0f * Scale));
	if (HaloMaterial)
	{
		HaloMaterial->SetScalarParameterValue(TEXT("Fade"), Brightness);
	}
	Core->SetVisibility(!bHaloOnly && Brightness > 0.05f);
	Light->SetIntensity(BaseLight * Brightness);
}

// ---- AAetheriaLaserBolt --------------------------------------------------------------------------

AAetheriaLaserBolt::AAetheriaLaserBolt()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LaserGlow(TEXT("/Game/Aetheria/Materials/M_LaserGlow.M_LaserGlow"));

	Core = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Core"));
	RootComponent = Core;
	AetheriaWeaponVFX::SetupMesh(Core, Cylinder.Object, Emissive.Object);

	Glow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Core);
	Glow->SetRelativeScale3D(FVector(4.0f, 4.0f, 1.25f));
	AetheriaWeaponVFX::SetupMesh(Glow, Cylinder.Object, LaserGlow.Object);
}

void AAetheriaLaserBolt::Setup(const FVector& Start, const FVector& End, const FLinearColor& Color, float Width, float Speed)
{
	Target = End;
	Direction = (End - Start).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
	BoltColor = Color;
	BoltSpeed = Speed;
	BoltWidth = Width;

	// a slim core one-third the bolt's width, eight widths long
	const float CoreWidth = Width * 0.33f;
	SetActorLocationAndRotation(Start, FRotationMatrix::MakeFromZ(Direction).Rotator());
	SetActorScale3D(FVector(CoreWidth / 100.0f, CoreWidth / 100.0f, Width * 8.0f / 100.0f));

	CoreMaterial = Core->CreateDynamicMaterialInstance(0);
	GlowMaterial = Glow->CreateDynamicMaterialInstance(0);
	if (CoreMaterial)
	{
		CoreMaterial->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(Color, FLinearColor::White, 0.7f));
		CoreMaterial->SetScalarParameterValue(TEXT("Glow"), 40.0f);
	}
	if (GlowMaterial)
	{
		GlowMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		GlowMaterial->SetScalarParameterValue(TEXT("Glow"), 14.0f);
	}

	// never outlive a missed target
	SetLifeSpan(FVector::Dist(Start, End) / FMath::Max(Speed, 1.0f) + 0.5f);
}

AAetheriaLaserBolt* AAetheriaLaserBolt::Spawn(UWorld* World, const FVector& Start, const FVector& End,
	const FLinearColor& Color, float Width, float Speed)
{
	if (!World)
	{
		return nullptr;
	}
	AAetheriaLaserBolt* Bolt = World->SpawnActor<AAetheriaLaserBolt>(AAetheriaLaserBolt::StaticClass(),
		FTransform(Start), AetheriaWeaponVFX::SpawnParams());
	if (Bolt)
	{
		Bolt->Setup(Start, End, Color, Width, Speed);
	}
	return Bolt;
}

void AAetheriaLaserBolt::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector Location = GetActorLocation();
	const float Step = BoltSpeed * DeltaSeconds;
	if (FVector::Dist(Location, Target) <= Step)
	{
		AAetheriaImpactFlash::Spawn(GetWorld(), Target, BoltColor, BoltWidth * 9.0f, 0.25f, 60.0f);
		AetheriaAudio::PlayAt(this, TEXT("S_Bolt_Impact"), Target, AetheriaAudio::ERange::Battle, 0.3f,
			FMath::FRandRange(0.85f, 1.15f), TEXT("Impact"));
		Destroy();
		return;
	}
	SetActorLocation(Location + Direction * Step);
}
