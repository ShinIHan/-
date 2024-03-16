#pragma once

#include "CoreMinimal.h"
#include "InteractionType.generated.h"

/**
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.23
 * ���۷��� : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */

// ���ͷ��� ��� Ÿ��
// ���ͷ��� ����
USTRUCT(BlueprintType, Category = "InteractionType")
struct PROJECT_TWO_API FInteractionInformation
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	UObject* InteractionObject;
	
	UPROPERTY(BlueprintReadWrite)
	FTransform RelativeTransform;
	
	//Original Settings
	UPROPERTY()
	bool bHighlightingObj;

	FORCEINLINE AActor* GetTargetActor() const
	{
		return Cast<AActor>(InteractionObject);
	}
	FORCEINLINE UPrimitiveComponent* GetTargetComponent() const
	{
		return Cast<UPrimitiveComponent>(InteractionObject);
	}

	FORCEINLINE bool operator==(const AActor* Other) const
	{
		if (Other && InteractionObject && InteractionObject == (const UObject*)Other)
			return true;

		return false;
	}
	FORCEINLINE bool operator==(const UPrimitiveComponent* Other) const
	{
		if (Other && InteractionObject && InteractionObject == (const UObject*)Other)
			return true;

		return false;
	}

	FORCEINLINE bool operator==(const UObject* Other) const
	{
		if (Other && InteractionObject == Other)
			return true;

		return false;
	}

	FInteractionInformation() :
		InteractionObject(nullptr),
		RelativeTransform(FTransform::Identity), bHighlightingObj(true)
	{
	}
};
