#pragma once

#include "CoreMinimal.h"
#include "InteractionType.generated.h"

/**
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.23
 * 레퍼런스 : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */

// 인터렉션 대상 타입
// 인터렉션 정보
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
