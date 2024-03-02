// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LBlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

	UPROPERTY()	
	TObjectPtr<UTexture2D> TopCrosshair;

	UPROPERTY()
	TObjectPtr<UTexture2D> BottomCrosshair;

	UPROPERTY()
	TObjectPtr<UTexture2D> LeftCrosshair;

	UPROPERTY()
	TObjectPtr<UTexture2D> RightCrosshair;

	UPROPERTY()
	TObjectPtr<UTexture2D> CenterCrosshair;

	UPROPERTY()
	float CrosshairSpread;

	UPROPERTY()
	FLinearColor CrosshairColor = FLinearColor::White;
};

/**
 * 
 */
UCLASS()
class LBLASTER_API ALBlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALBlasterHUD();
	virtual void DrawHUD() override;
	
	FORCEINLINE void SetHUDPackage(const FHUDPackage& InPackage) { HUDPackage = InPackage; }
	void SetHUDHealth(float InHealth, float InMaxHealth);
	void SetHUDScore(float InScore);
	void SetHUDDeath(int32 InDeath);
	void SetHUDAmmo(int32 InAmmo);
	void SetHUDCarriedAmmo(int32 InCarriedAmmo);
	void SetHUDWeaponTypeText(const FString& InWeaponTypeString);
	void SetHUDMatchCountdown(float InCountdownTime);
	void SetHUDAnnouncementCountdown(float InCountdownTime);
	void AddCharacterOverlay();
	void RemoveCharacterOverlay();
	void AddAnnouncement();
	void HideAnnouncement();
	void SetCooldownAnnouncement();

private:
	/*
	 *	Crosshair
	 */
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* InTexture, const FVector2D& InViewportCenter, const FVector2D& InSpread, const FLinearColor& InLinearColor);

	UPROPERTY(EditAnywhere, Category="LBlaster|Crosshair")
	float CrosshairSpreadMax;

	/*
	 *	Character Overlay
	 */
	UPROPERTY(EditAnywhere, Category="LBlaster|Character Overlay")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	TObjectPtr<class UCharacterOverlay> CharacterOverlay;

	/*
	 *	Announcement Overlay
	 */
	UPROPERTY(EditAnywhere, Category="LBlaster|Announcement Overlay")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	TObjectPtr<class UAnnouncement> Announcement;
};
