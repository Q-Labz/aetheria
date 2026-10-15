// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetheriaMoveMarker.generated.h"

class UStaticMeshComponent;

/**
 *  Flat cyan ring shown where a move order lands; removes itself after 1.5 s.
 *  BP_MoveMarker is a Blueprint child of this class.
 */
UCLASS(Blueprintable)
class AAetheriaMoveMarker : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Ring;

public:

	AAetheriaMoveMarker();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	FLinearColor Color = FLinearColor(0.0f, 1.0f, 1.0f);

protected:

	virtual void BeginPlay() override;
};
