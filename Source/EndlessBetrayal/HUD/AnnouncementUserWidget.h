// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API UAnnouncementUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* WarmUpTimeText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Player Stats")
	class UTextBlock* InfoText;
};
