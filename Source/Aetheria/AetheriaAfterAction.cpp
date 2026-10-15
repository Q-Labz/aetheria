// Copyright Epic Games, Inc. All Rights Reserved.

#include "AetheriaAfterAction.h"
#include "AetheriaCombatManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UAetheriaAfterAction::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrimaryButton)
	{
		PrimaryButton->OnClicked.AddUniqueDynamic(this, &UAetheriaAfterAction::HandlePrimary);
	}
	if (SecondaryButton)
	{
		SecondaryButton->OnClicked.AddUniqueDynamic(this, &UAetheriaAfterAction::HandleSecondary);
	}
	ApplyLabels();
}

void UAetheriaAfterAction::Setup(AAetheriaCombatManager* InManager, bool bInVictory, const FText& Title, const FText& Body)
{
	Manager = InManager;
	bVictory = bInVictory;

	if (TitleText)
	{
		TitleText->SetText(Title);
	}
	if (BodyText)
	{
		BodyText->SetText(Body);
	}
	ApplyLabels();
}

void UAetheriaAfterAction::ApplyLabels()
{
	// victory: [Return to Command] ; defeat: [Retry Engagement] [Return to Command]
	if (PrimaryLabel)
	{
		PrimaryLabel->SetText(FText::FromString(bVictory ? TEXT("Return to Command") : TEXT("Retry Engagement")));
	}
	if (SecondaryLabel)
	{
		SecondaryLabel->SetText(FText::FromString(TEXT("Return to Command")));
	}
	if (SecondaryButton)
	{
		SecondaryButton->SetVisibility(bVictory ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UAetheriaAfterAction::HandlePrimary()
{
	if (!Manager)
	{
		return;
	}

	if (bVictory)
	{
		Manager->ReturnToCommand();
	}
	else
	{
		Manager->RetryEngagement();
	}
}

void UAetheriaAfterAction::HandleSecondary()
{
	if (Manager)
	{
		Manager->ReturnToCommand();
	}
}
