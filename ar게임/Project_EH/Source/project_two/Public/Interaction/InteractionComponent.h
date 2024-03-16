#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractionType.h"
#include "Components/SceneComponent.h"
#include "InteractionComponent.generated.h"

/**
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.23
 * ���۷��� : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvent, UObject*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTickOnEvent, UObject*, Target, float, Delta);

UCLASS(meta=(BlueprintSpawnableComponent), ClassGroup = VRSystem)
class PROJECT_TWO_API UInteractionComponent : public USceneComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:	
	UInteractionComponent();
	virtual void HandleDetection();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Call Back
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FOnEvent OnExec;
	
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FOnEvent OnEndExec;
	
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FOnEvent OnDetect;
	
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FOnEvent OnEndDetect;
	
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FTickOnEvent TickOnDetect;
	
	UPROPERTY(BlueprintAssignable, Category = "InteractionComponent|InteractionEvent")
	FTickOnEvent TickOnExec;

	// ���ͷ��� �Է� �Լ�
	UFUNCTION(BlueprintCallable, Category = "InteractionComponent")
	virtual bool ExecuteInteraction();
	
	UFUNCTION(BlueprintCallable, Category = "InteractionComponent")
	virtual bool EndExecuteInteraction();

	//���� �Լ�
	UFUNCTION()
	virtual UObject* DetectTarget();
	
	UFUNCTION()
	virtual bool GetDenyHighlight(UObject* Object);
	
	UFUNCTION()
	virtual bool GetDenyInteraction(UObject* Object);
	
	UFUNCTION()
	virtual void HighlightTarget(UObject* Object);
	
	UFUNCTION()
	virtual void ClearHighlightComps();

protected:
	UPROPERTY()
		UClass* InterfaceStaticClass;

	UPROPERTY()
		bool IsExecuting = false;

	UPROPERTY()
		UObject* TargetObject;

	UPROPERTY()
		TArray<UPrimitiveComponent*> HighlightedComps;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	bool bIsDebug = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	float TraceDistance = 1000;
};
