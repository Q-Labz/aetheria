// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaBeamVFX.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaBeamVFX::AAetheriaBeamVFX()
{
	PrimaryActorTick.bCanEverTick = false;

	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	RootComponent = Beam;
	Beam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Beam->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder.Succeeded())
	{
		Beam->SetStaticMesh(Cylinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	if (Emissive.Succeeded())
	{
		Beam->SetMaterial(0, Emissive.Object);
	}
}

void AAetheriaBeamVFX::Setup(FVector Start, FVector End, float Width, FLinearColor Color, float Life)
{
	const FVector Span = End - Start;

	// the engine cylinder is 100 uu tall and 100 uu across, centred on its pivot
	SetActorLocationAndRotation((Start + End) * 0.5f, FRotationMatrix::MakeFromZ(Span).Rotator());
	SetActorScale3D(FVector(Width / 100.0f, Width / 100.0f, FMath::Max(Span.Size(), 1.0f) / 100.0f));

	if (!BeamMaterial)
	{
		BeamMaterial = Beam->CreateDynamicMaterialInstance(0);
	}
	if (BeamMaterial)
	{
		BeamMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}

	if (Life > 0.0f)
	{
		SetLifeSpan(Life);
	}
}
