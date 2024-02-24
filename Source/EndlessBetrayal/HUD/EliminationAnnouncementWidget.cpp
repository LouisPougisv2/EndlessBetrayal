// Fill out your copyright notice in the Description page of Project Settings.


#include "EliminationAnnouncementWidget.h"

#include "Components/TextBlock.h"

void UEliminationAnnouncementWidget::SetEliminationAnnouncementText(FString AttackerName, FString VictimName)
{
	FString EliminationAnnouncementText = FString::Printf(TEXT("%s eliminated %s !"), *AttackerName, *VictimName);
	if(AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(EliminationAnnouncementText));
	}
}
