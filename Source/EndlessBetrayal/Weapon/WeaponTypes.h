﻿#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(Displayname = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA (DisplayName = "Pistol"),
	EWT_SMG UMETA (DisplayName = "SMG"),

	EWT_MAX UMETA(Displayname = "DefaultMAX")
};