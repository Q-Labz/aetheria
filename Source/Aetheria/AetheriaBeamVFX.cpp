// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaBeamVFX.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace AetheriaVFX
{
	/** Glow sheath is this much wider than the beam; the core this much narrower */
	constexpr float GlowScale = 1.8f;
	constexpr float CoreScale = 0.35f;
}

AAetheriaBeamVFX::AAetheriaBeamVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LaserGlow(TEXT("/Game/Aetheria/Materials/M_LaserGlow.M_LaserGlow"));

	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	RootComponent = Beam;

	Glow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Beam);

	for (UStaticMeshComponent* Mesh : { Beam.Get(), Glow.Get() })
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		if (Cylinder.Succeeded())
		{
			Mesh->SetStaticMesh(Cylinder.Object);
		}
	}
	if (Emissive.Succeeded())
	{
		Beam->SetMaterial(0, Emissive.Object);
	}
	if (LaserGlow.Succeeded())
	{
		Glow->SetMaterial(0, LaserGlow.Object);
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Beam);
	Light->SetCastShadows(false);
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->SetVisibility(false);
}

void AAetheriaBeamVFX::Setup(FVector Start, FVector End, float Width, FLinearColor Color, float Life)
{
	const FVector Span = End - Start;
	const float Length = FMath::Max(Span.Size(), 1.0f);

	// the engine cylinder is 100 uu tall and 100 uu across, centred on its pivot; the actor's
	// scale carries the core, and the sheath is scaled relative to it
	SetActorLocationAndRotation((Start + End) * 0.5f, FRotationMatrix::MakeFromZ(Span).Rotator());
	const float CoreWidth = Width * AetheriaVFX::CoreScale;
	SetActorScale3D(FVector(CoreWidth / 100.0f, CoreWidth / 100.0f, Length / 100.0f));
	Glow->SetRelativeScale3D(FVector(AetheriaVFX::GlowScale / AetheriaVFX::CoreScale, AetheriaVFX::GlowScale / AetheriaVFX::CoreScale, 1.0f));

	if (!CoreMaterial)
	{
		CoreMaterial = Beam->CreateDynamicMaterialInstance(0);
	}
	if (!GlowMaterial)
	{
		GlowMaterial = Glow->CreateDynamicMaterialInstance(0);
	}

	// a near-white core tinted toward the beam colour, inside a coloured sheath
	if (CoreMaterial)
	{
		CoreMaterial->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(Color, FLinearColor::White, 0.65f));
		CoreMaterial->SetScalarParameterValue(TEXT("Glow"), 40.0f);
	}
	BaseGlow = FMath::Clamp(Width / 20.0f, 6.0f, 20.0f);
	if (GlowMaterial)
	{
		GlowMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		GlowMaterial->SetScalarParameterValue(TEXT("Glow"), BaseGlow);
		GlowMaterial->SetScalarParameterValue(TEXT("Fade"), 1.0f);
	}

	const bool bLit = Width >= LightMinWidth;
	Light->SetVisibility(bLit);
	if (bLit)
	{
		BaseLight = LightIntensity * (Width / LightMinWidth);
		Light->SetLightColor(Color);
		Light->SetIntensity(BaseLight);
		// world-space radius, so undo the actor's scale on the component
		Light->SetAttenuationRadius(FMath::Min(Length * 0.6f, 6000.0f) + Width * 6.0f);
		Light->SetWorldScale3D(FVector::OneVector);
	}

	BaseWidth = CoreWidth;
	TotalLife = Life;
	Age = 0.0f;
	if (Life > 0.0f)
	{
		SetLifeSpan(Life);
		SetActorTickEnabled(true);
	}
}

void AAetheriaBeamVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TotalLife <= 0.0f)
	{
		return;
	}

	// hold briefly, then narrow and fade
	Age += DeltaSeconds;
	const float Remaining = 1.0f - FMath::Clamp(Age / TotalLife, 0.0f, 1.0f);
	const float Fade = FMath::Clamp(Remaining * 1.6f, 0.0f, 1.0f);

	FVector Scale = GetActorScale3D();
	Scale.X = Scale.Y = BaseWidth / 100.0f * FMath::Lerp(0.3f, 1.0f, Fade);
	SetActorScale3D(Scale);

	if (GlowMaterial)
	{
		GlowMaterial->SetScalarParameterValue(TEXT("Fade"), Fade);
	}
	if (Light->IsVisible())
	{
		Light->SetIntensity(BaseLight * Fade);
	}
}
