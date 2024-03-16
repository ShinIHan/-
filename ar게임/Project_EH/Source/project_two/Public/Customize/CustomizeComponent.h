// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CustomizeComponent.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Clay
};

UENUM(BlueprintType)
enum class ECustomizeCategory : uint8
{
	Type = 0,
	Body = 1,
	Hair = 2,
	Top = 3,
	Bottom = 4,
	Shoes = 5
};

USTRUCT(BlueprintType, Category = "Customize")
struct PROJECT_TWO_API FDisplayData
{
	GENERATED_USTRUCT_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
		FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
		FString Description;
};

USTRUCT(BlueprintType, Category = "Customize")
struct PROJECT_TWO_API FCustomizeItemData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	ECustomizeCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	FDisplayData DisplayData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	TArray<UMaterialInstance*> Materials;
};

USTRUCT(BlueprintType, Category = "Customize")
struct PROJECT_TWO_API FCustomizeRepData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	ECharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	TArray<int32> Items;
};

UCLASS(Blueprintable, ClassGroup = (Customize), meta = (BlueprintSpawnableComponent))
class PROJECT_TWO_API UCustomizeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomizeComponent();

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	bool GetCustomizeItemData(const int32& ItemNum, FCustomizeItemData& ItemData);

	UFUNCTION(BlueprintCallable)
	UDataTable* GetDataTableByCharacterType();

	UFUNCTION(BlueprintCallable)
	void SetCharacterType(ECharacterType CharacterType);

	UFUNCTION(BlueprintCallable)
	void SetCustomizeRepData(TArray<int32> Items, bool bIncludeType);

	UFUNCTION(BlueprintCallable)
	void ResetCustomize();

	UFUNCTION(BlueprintCallable)
	void CustomizeNumMapsToRepData();

	UFUNCTION()
	USkeletalMeshComponent* AddSkeletalMeshComponent(ECustomizeCategory Category);

	UFUNCTION(BlueprintCallable)
	void ApplyCustomizeItem(const int32& ItemNum);

	//데이터 테이블에서 아이템 정보를 가져와 적용 TMap에 있는 USkeletalMeshComponent에 적용
	UFUNCTION(BlueprintCallable)
	void ApplyCustomizeRepItems();

	// Original array state is useless without full serialize, it just hold last delta
	UFUNCTION()
	virtual void OnRep_CustomizeRepData() 
	{
		ApplyCustomizeRepItems();
	}

	UFUNCTION(BlueprintCallable)
	TArray<int32> GetCustomizeDBData();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Customize")
	USkeletalMeshComponent* CharacterMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	TMap<ECharacterType, UDataTable*> DataTableMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CustomizeRepData, Category = "Customize")
	FCustomizeRepData CustomizeRepData;

	UPROPERTY(BlueprintReadWrite, Category = "Customize")
	TMap<ECustomizeCategory, USkeletalMeshComponent*> SKMeshMaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	TMap<ECustomizeCategory, int32> CustomizeNumMaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	bool bNoRenderInMainPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customize")
	bool bCustomizeLevel = false;

	UPROPERTY()
	FTimerHandle Rep_TimerHandle;
};
