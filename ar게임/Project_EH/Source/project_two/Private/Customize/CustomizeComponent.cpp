// Fill out your copyright notice in the Description page of Project Settings.


#include "Customize/CustomizeComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstance.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCustomizeComponent::UCustomizeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCustomizeComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCustomizeComponent, CustomizeRepData);
}

// Called when the game starts
void UCustomizeComponent::BeginPlay()
{
	if (!CharacterMeshComponent)
	{
		CharacterMeshComponent = Cast<ACharacter>(GetOwner())->GetMesh();
	}

	Super::BeginPlay();
}

bool UCustomizeComponent::GetCustomizeItemData(const int32& ItemNum, FCustomizeItemData& ItemData)
{
	UDataTable* DataTable = GetDataTableByCharacterType();
	FString ItemStr = FString::FromInt(ItemNum);
	if (FCustomizeItemData* ItemDataRef = DataTable->FindRow<FCustomizeItemData>(FName(*ItemStr), ""))
	{
		ItemData = *ItemDataRef;
		return true;
	}
	return false;

}

USkeletalMeshComponent* UCustomizeComponent::AddSkeletalMeshComponent(ECustomizeCategory Category)
{
	if (!CharacterMeshComponent)
		CharacterMeshComponent = Cast<ACharacter>(GetOwner())->GetMesh();

	FString Name;
	//const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECustomizeCategory"), true);
	const UEnum* enumPtr = FindObject<UEnum>(nullptr, *FString("/Script/project_two.ECustomizeCategory"), true);

	if (!enumPtr)
	{
		Name = FString("CustomizeSkeletal");
	}
	Name = enumPtr->GetNameStringByIndex(static_cast<int32>(Category));

	USkeletalMeshComponent* CustomizeSkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), FName(*Name));
	if (CustomizeSkeletalMeshComponent != nullptr)
	{
		CustomizeSkeletalMeshComponent->RegisterComponent();
		CustomizeSkeletalMeshComponent->AttachToComponent(CharacterMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	SKMeshMaps.Add(Category, CustomizeSkeletalMeshComponent);

	return CustomizeSkeletalMeshComponent;
}



UDataTable* UCustomizeComponent::GetDataTableByCharacterType()
{
	if (UDataTable** DataTable = DataTableMap.Find(CustomizeRepData.CharacterType))
		return *DataTable;
	return DataTableMap.begin()->Value;
}



void UCustomizeComponent::ResetCustomize()
{
	CustomizeRepData.Items.Empty();
	CustomizeNumMaps.Empty();
	TArray<USkeletalMeshComponent*> SkMeshComps;
	SKMeshMaps.GenerateValueArray(SkMeshComps);
	for (USkeletalMeshComponent* SKMeshComp : SkMeshComps)
	{
		SKMeshComp->SetSkeletalMesh(nullptr);
	}
}

void UCustomizeComponent::CustomizeNumMapsToRepData()
{
	CustomizeRepData.Items.Empty();
	CustomizeNumMaps.GenerateValueArray(CustomizeRepData.Items);
}

void UCustomizeComponent::SetCharacterType(ECharacterType CharacterType)
{
	CustomizeRepData.CharacterType = CharacterType;
}

void UCustomizeComponent::ApplyCustomizeItem(const int32& ItemNum)
{

	FCustomizeItemData ItemData;
	if (!GetCustomizeItemData(ItemNum, ItemData))
	{
		return;
	}

	USkeletalMeshComponent* SKMeshComp;
	if (ItemData.Category == ECustomizeCategory::Body)
	{
		if (!CharacterMeshComponent)
			CharacterMeshComponent = Cast<ACharacter>(GetOwner())->GetMesh();
		SKMeshComp = CharacterMeshComponent;
	}
	else
	{
		if (auto SKMesh = SKMeshMaps.Find(ItemData.Category))
			SKMeshComp = *(SKMesh);
		else
			SKMeshComp = AddSkeletalMeshComponent(ItemData.Category);
	}
	
	if (!SKMeshComp)
		return;
	
	SKMeshComp->SetSkeletalMesh(ItemData.SkeletalMesh);
	for (int32 i = 0; i != ItemData.Materials.Num(); ++i)
	{
		if (ItemData.Materials[i])
			SKMeshComp->SetMaterial(i, ItemData.Materials[i]);

		//SKMeshComp->bOwnerNoSee = true;
	}
	if (SKMeshComp != CharacterMeshComponent)
		SKMeshComp->SetLeaderPoseComponent(CharacterMeshComponent);

	//if (ItemData.Category == ECustomizeCategory::Hair)
		//SKMeshComp->bRenderInMainPass = false;

	CustomizeNumMaps.Add(ItemData.Category, ItemNum);
}

void UCustomizeComponent::ApplyCustomizeRepItems()
{
	if (!CharacterMeshComponent)
		CharacterMeshComponent = Cast<ACharacter>(GetOwner())->GetMesh();

	if (!CharacterMeshComponent)
	{
		GetWorld()->GetTimerManager().SetTimer(Rep_TimerHandle, this, &UCustomizeComponent::ApplyCustomizeRepItems, 3.0f, false);
		return;
	}
	//Item Rep Data 순환.
	for (int32 ItemNum : CustomizeRepData.Items)
	{
		//Item Data로 적용
		ApplyCustomizeItem(ItemNum);
	}

	GetWorld()->GetTimerManager().ClearTimer(Rep_TimerHandle);

	if (bNoRenderInMainPass)
	{
		if (GetOwner()->HasNetOwner())
		{
			TArray<USkeletalMeshComponent*> SKMeshes;
			SKMeshMaps.GenerateValueArray(SKMeshes);
			for (USkeletalMeshComponent* SkMesh : SKMeshes)
			{
				SkMesh->SetOwnerNoSee(true);
			}
		}
	}
	else
	{
		// Customize Level이면 작동하지 않음
		if (!bCustomizeLevel && GetOwner()->HasNetOwner())
		{
			USkeletalMeshComponent* SKHairMesh;
			if (SKMeshMaps.Find(ECustomizeCategory::Hair) != nullptr)
			{
				SKHairMesh = *SKMeshMaps.Find(ECustomizeCategory::Hair);
				SKHairMesh->SetOwnerNoSee(true);
				SKHairMesh->bCastHiddenShadow = true;
			}
		}
	}
}



// Call Only Server Using DB
void UCustomizeComponent::SetCustomizeRepData(TArray<int32> Items, bool bIncludeType)
{
	if (bIncludeType)
	{
		Items.Sort();
		if (Items[0] < 1000)
		{
			CustomizeRepData.CharacterType = static_cast<ECharacterType>(Items[0]);
			Items.RemoveAt(0);
		}
	}
	CustomizeRepData.Items = Items;
}

TArray<int32> UCustomizeComponent::GetCustomizeDBData()
{
	TArray<int32> Data = CustomizeRepData.Items;
	Data.Add(static_cast<int32>(CustomizeRepData.CharacterType));

	return Data;
}