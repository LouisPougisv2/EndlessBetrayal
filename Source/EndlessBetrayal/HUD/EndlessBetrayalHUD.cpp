// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalHUD.h"

#include "CharacterOverlay.h"
#include "Blueprint/UserWidget.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"

void AEndlessBetrayalHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewPortSize;
	if(IsValid(GetWorld()) && IsValid(GetWorld()->GetGameViewport()))
	{
		GetWorld()->GetGameViewport()->GetViewportSize(ViewPortSize);
	}

	float SpreadScaled = CrosshairSpreadMax * HUDTextures.CrosshairSpreadFactor;
	
	const FVector2d ViewportCenter(ViewPortSize.X / 2, ViewPortSize.Y / 2);
	if(IsValid(HUDTextures.CrosshairCenter)) DrawCrosshair(HUDTextures.CrosshairCenter, ViewportCenter, FVector2d(0.0f, 0.0f), HUDTextures.CrosshairColor);
	if(IsValid(HUDTextures.CrosshairTop)) DrawCrosshair(HUDTextures.CrosshairTop, ViewportCenter, FVector2d(0.0f, -SpreadScaled), HUDTextures.CrosshairColor);
	if(IsValid(HUDTextures.CrosshairRight)) DrawCrosshair(HUDTextures.CrosshairRight, ViewportCenter, FVector2d(SpreadScaled, 0.0f), HUDTextures.CrosshairColor);
	if(IsValid(HUDTextures.CrosshairBottom)) DrawCrosshair(HUDTextures.CrosshairBottom, ViewportCenter, FVector2d(0.0f, SpreadScaled), HUDTextures.CrosshairColor);
	if(IsValid(HUDTextures.CrosshairLeft)) DrawCrosshair(HUDTextures.CrosshairLeft, ViewportCenter, FVector2d(-SpreadScaled, 0.0f), HUDTextures.CrosshairColor);
	
}

void AEndlessBetrayalHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void AEndlessBetrayalHUD::AddCharacterOverlay()
{
	AEndlessBetrayalPlayerController* PlayerController = Cast<AEndlessBetrayalPlayerController>(GetOwningPlayerController());
	if(IsValid(PlayerController) && IsValid(CharacterOverlayClass))
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AEndlessBetrayalHUD::DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.0f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.0f) + Spread.Y);
	
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.0f, 0.0f, 1.0f, 1.0f, CrosshairColor);
}
