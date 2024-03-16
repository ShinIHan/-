#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractionComponent.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.23
 * 레퍼런스 : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */
class PROJECT_TWO_API IInteractionInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	/** Settings **/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction", meta = (DisplayName = "IsDenyHighlight"))
		bool DenyHighlighting(UInteractionComponent* InteractionComponent);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction", meta = (DisplayName = "IsDenyInteraction"))
		bool DenyInteraction(UInteractionComponent* InteractionComponent);

	/** Main Implement **/

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
		void OnInteract(UInteractionComponent* InteractionComponent);
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
		void OnEndInteract(UInteractionComponent* InteractionComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
		void TickOnInteract(UInteractionComponent* InteractionComponent, float DeltaTime);
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
		void TickOnDetected(UInteractionComponent* InteractionComponent, float DeltaTime);
};
