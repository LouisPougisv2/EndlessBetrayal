// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EliminationAnnouncementWidget.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API UEliminationAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetEliminationAnnouncementText(FString AttackerName, FString VictimName);

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;
};
