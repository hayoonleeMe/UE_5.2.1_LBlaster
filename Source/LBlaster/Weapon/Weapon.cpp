// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"

#include "Casing.h"
#include "LBlaster.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/LBlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Player/LBlasterPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	/* Mesh */
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_FootPlacement, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* Custom Depth */
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
    WeaponMesh->MarkRenderStateDirty();	// 변경 사항을 Refresh
	EnableCustomDepth(true);

	/* Overlap Sphere */
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* Pickup Widget Component */
	PickupWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget Component"));
	PickupWidgetComponent->SetupAttachment(RootComponent);
	PickupWidgetComponent->SetRelativeLocation(FVector(0.f, 20.f, 50.f));
	PickupWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	PickupWidgetComponent->SetDrawAtDesiredSize(true);
	PickupWidgetComponent->SetVisibility(false);
	LocOffset = FVector(0.f, 20.f, 50.f);

	static ConstructorHelpers::FClassFinder<UUserWidget> PickupWidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/LBlaster/UI/HUD/WBP_PickupWidget.WBP_PickupWidget_C'"));
	if (PickupWidgetClassRef.Class)
	{
		PickupWidgetComponent->SetWidgetClass(PickupWidgetClassRef.Class);
	}

	/* Crosshair */
	static ConstructorHelpers::FObjectFinder<UTexture2D> CenterCrosshairRef(TEXT("/Script/Engine.Texture2D'/Game/LBlaster/UI/Crosshair/Primary/Crosshair_Center.Crosshair_Center'"));
	if (CenterCrosshairRef.Object)
	{
		CenterCrosshair = CenterCrosshairRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> TopCrosshairRef(TEXT("/Script/Engine.Texture2D'/Game/LBlaster/UI/Crosshair/Primary/Crosshair_Top.Crosshair_Top'"));
	if (TopCrosshairRef.Object)
	{
		TopCrosshair = TopCrosshairRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> BottomCrosshairRef(TEXT("/Script/Engine.Texture2D'/Game/LBlaster/UI/Crosshair/Primary/Crosshair_Bottom.Crosshair_Bottom'"));
	if (BottomCrosshairRef.Object)
	{
		BottomCrosshair = BottomCrosshairRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> LeftCrosshairRef(TEXT("/Script/Engine.Texture2D'/Game/LBlaster/UI/Crosshair/Primary/Crosshair_Left.Crosshair_Left'"));
	if (LeftCrosshairRef.Object)
	{
		LeftCrosshair = LeftCrosshairRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> RightCrosshairRef(TEXT("/Script/Engine.Texture2D'/Game/LBlaster/UI/Crosshair/Primary/Crosshair_Right.Crosshair_Right'"));
	if (RightCrosshairRef.Object)
	{
		RightCrosshair = RightCrosshairRef.Object;
	}

	/* Auto Fire */
	bAutomatic = true;
	FireDelay = 0.15f;

	/* Damage */
	Damage = 20.f;
	HeadshotMultiplier = 1.5f;

	/* ADS FOV */
	ADSFOV = 70.f;

	/* Attach Transform */
	AttachTransform = FTransform::Identity;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::ShowPickupWidget(bool bInShow) const
{
	if (PickupWidgetComponent)
	{
		// 항상 Weapon 위에 표시되도록 위치 보정
		const FVector RelativeLocation = GetActorRotation().UnrotateVector(LocOffset);
		PickupWidgetComponent->SetRelativeLocation(RelativeLocation);
		PickupWidgetComponent->SetVisibility(bInShow);
	}
}

void AWeapon::SetWeaponState(EWeaponState InWeaponState)
{
	WeaponState = InWeaponState;
	OnChangedWeaponState();
}

void AWeapon::SetHUDAmmo()
{
	if (IsValidOwnerCharacter())
	{
		OwnerCharacter->SetHUDAmmo(Ammo);
	}
}

void AWeapon::AddAmmo(int32 InAmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + InAmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(InAmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 InAmmoToAdd)
{
	if (HasAuthority())
	{
		return;
	}

	Ammo = FMath::Clamp(Ammo + InAmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		if (const USkeletalMeshSocket* Socket = WeaponMesh->GetSocketByName(TEXT("AmmoEject")))
		{
			const FTransform SocketTransform = Socket->GetSocketTransform(WeaponMesh);
			
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}

	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	const FDetachmentTransformRules DetachRule(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRule);
	SetOwner(nullptr);
}

void AWeapon::Holstered()
{
	SetWeaponState(EWeaponState::EWS_Initial);

	const FDetachmentTransformRules DetachRule(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRule);
	SetOwner(nullptr);
}

void AWeapon::EnableCustomDepth(bool bEnable) const
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Overlap Event 판정, 실제 무기 착용은 서버에서만 이루어지게 하고, Pickup Widget은 로컬에서 표시해 높은 핑에서도 바로 볼 수 있게 함.
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner())
	{
		SetHUDAmmo();
	}
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                   bool bFromSweep, const FHitResult& SweepResult)
{
	if (ALBlasterCharacter* OtherCharacter = Cast<ALBlasterCharacter>(OtherActor))
	{
		OtherCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ALBlasterCharacter* OtherCharacter = Cast<ALBlasterCharacter>(OtherActor))
	{
		OtherCharacter->SetOverlappingWeapon(nullptr);
	}
}

bool AWeapon::IsValidOwnerCharacter()
{
	if (!OwnerCharacter && GetOwner())
	{
		OwnerCharacter = Cast<ALBlasterCharacter>(GetOwner());
	}
	return OwnerCharacter != nullptr;
}

bool AWeapon::IsValidOwnerController()
{
	if (!OwnerController && IsValidOwnerCharacter() && OwnerCharacter->GetController())
	{
		OwnerController = Cast<ALBlasterPlayerController>(OwnerCharacter->GetController());
	}
	return OwnerController != nullptr;
}

void AWeapon::OnRep_WeaponState()
{
	OnChangedWeaponState();
}

void AWeapon::OnChangedWeaponState()
{
	// TODO : 모든 EWeaponState 경우 처리 필요
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		// 무기가 장착된 상태라면 Pickup Widget을 숨기고 Pickup Overlap 발생 중지
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EnableCustomDepth(false);
		break;
		
	case EWeaponState::EWS_Dropped:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		EnableCustomDepth(true);
		break;
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp<int32>(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if (IsValidOwnerCharacter() && OwnerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 InServerAmmo)
{
	if (HasAuthority())
	{
		return;
	}

	Ammo = InServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}
