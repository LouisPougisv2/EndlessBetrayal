// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "BulletCasing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();	//Forcing the PP refresh
	ToggleCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	if (HasAuthority()) //Same as if checking GetLocalRole() == ENetRole::ROLE_Authority
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlapBegin);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereOverlapEnd);

	} 
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		WeaponOwnerCharacter = nullptr;
		WeaponOwnerController = nullptr;
	}
	else
	{
		if(IsValid(WeaponOwnerCharacter) && IsValid(WeaponOwnerCharacter->GetEquippedWeapon()) && (WeaponOwnerCharacter->GetEquippedWeapon() == this))
		{
			UpdateHUDAmmo();
		}
	}
	
}


void AWeapon::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEndlessBetrayalCharacter* Character = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if (Character)
	{
		if(WeaponType == EWeaponType::EWT_Flag && Character->GetTeam() != Team) return;
		Character->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEndlessBetrayalCharacter* Character = Cast<AEndlessBetrayalCharacter>(OtherActor);
	if (Character)
	{
		if(WeaponType == EWeaponType::EWT_Flag && Character->GetTeam() != Team) return;
		Character->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	WeaponState = NewState;

	OnWeaponStateSet();
}

void AWeapon::SetWeaponMesh(USkeletalMeshComponent* InWeapon)
{
	WeaponMesh = InWeapon;
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		HandleWeaponEquipped();
		break;

	case EWeaponState::EWS_EquippedSecondary:
		HandleWeaponEquippedSecondary();
		break;
		
	case EWeaponState::EWS_Dropped:
		HandleWeaponDropped();
		break;

	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::HandleWeaponEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToggleCustomDepth(false);

	if(GetWeaponType() == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetEnableGravity(true);
	}

	BindOrRemovePingTooHighDelegate();
}

void AWeapon::HandleWeaponDropped()
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	ToggleCustomDepth(true);

	BindOrRemovePingTooHighDelegate();
}

void AWeapon::HandleWeaponEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ToggleCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();

	if(GetWeaponType() == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetEnableGravity(true);
	}

	BindOrRemovePingTooHighDelegate();
	
}

void AWeapon::UpdateHUDAmmo()
{
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<AEndlessBetrayalCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if(IsValid(WeaponOwnerCharacter))
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if(WeaponOwnerController)
		{
			WeaponOwnerController->UpdateWeaponAmmo(AmmoAmount);
		}
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority()) return;
	
	AmmoAmount = ServerAmmo;
	--SequenceNumber;

	//Correction (SequenceNumber represents the number of round spent but not replicated back to us yet
	AmmoAmount -= SequenceNumber;
	UpdateHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 ServerAmmoToAdd)
{
	if(HasAuthority()) return;
	
	AmmoAmount = FMath::Clamp(AmmoAmount + ServerAmmoToAdd, 0, MagCapacity);
	UpdateHUDAmmo();
}

void AWeapon::SpendRound()
{
	AmmoAmount = FMath::Clamp(--AmmoAmount, 0, MagCapacity);
	UpdateHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(AmmoAmount);
	}
	else if (WeaponOwnerCharacter && WeaponOwnerCharacter->IsLocallyControlled())
	{
		++SequenceNumber;
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::BindOrRemovePingTooHighDelegate()
{
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<AEndlessBetrayalCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if(IsValid(WeaponOwnerCharacter))
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if(WeaponOwnerController && HasAuthority())
		{
			if(!WeaponOwnerController->HighPingDelegate.IsBound())
			{
				WeaponOwnerController->HighPingDelegate.AddUniqueDynamic(this, &AWeapon::OnPingTooHigh);
			}
			else if(WeaponOwnerController->HighPingDelegate.IsBound())
			{
				WeaponOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
	}
}

void AWeapon::UpdateAmmo(int32 AmmoAmountToAdd)
{
	AmmoAmount = FMath::Clamp(AmmoAmount + AmmoAmountToAdd, 0, MagCapacity);
	UpdateHUDAmmo();
	ClientAddAmmo(AmmoAmount);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if(IsValid(WeaponMesh) && IsValid(FireAnimation))
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	
	if(IsValid(BulletCasingClass))
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if(IsValid(AmmoEjectSocket))
		{
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if(IsValid(World))
			{
				World->SpawnActor<ABulletCasing>(BulletCasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	//Updating Ammo
	SpendRound();
}

bool AWeapon::IsFullyLoaded()
{
	return AmmoAmount == MagCapacity;
}

void AWeapon::OnWeaponDropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	WeaponOwnerCharacter = nullptr;
	WeaponOwnerController = nullptr;
}

FVector AWeapon::GetTraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	if(!IsValid(MuzzleFlashSocket)) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SphereRadius);
	const FVector EndLocation = SphereCenter + RandVect;
	const FVector ToEndLocation = EndLocation - Start;
	
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::White, true);
	//DrawDebugSphere(GetWorld(), EndLocation, 4.0f, 12, FColor::Red, true);
	//DrawDebugLine(GetWorld(), TraceStartLocation, FVector(TraceStartLocation + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::White, true);
	
	return FVector(Start + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}

void AWeapon::ToggleCustomDepth(bool bEnable)
{
	if(IsValid(WeaponMesh))
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}
