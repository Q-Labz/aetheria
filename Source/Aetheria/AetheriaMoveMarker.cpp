// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaMoveMarker.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AAetheriaMoveMarker::AAetheriaMoveMarker()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 1.5f;

	Ring = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring"));
	RootComponent = Ring;
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ring->SetCastShadow(false);
	Ring->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.02f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder.Succeeded())
	{
		Ring->SetStaticMesh(Cylinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Emissive(TEXT("/Game/Aetheria/Materials/M_Emissive.M_Emissive"));
	if (Emissive.Succeeded())
	{
		Ring->SetMaterial(0, Emissive.Object);
	}
}

void AAetheriaMoveMarker::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* Material = Ring->CreateDynamicMaterialInstance(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
}
