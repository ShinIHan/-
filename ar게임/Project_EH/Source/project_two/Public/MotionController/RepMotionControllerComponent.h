// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utillity/VRSystemDataType.h"
#include "MotionControllerComponent.h"
#include "RepMotionControllerComponent.generated.h"

/*
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.17 
 * ���۷��� : VRExpansionPlugin, Project_One
 */

// UCLASS �������� ������ Blueprint Component â���� �߰��� �� ����
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRSystem)
class PROJECT_TWO_API URepMotionControllerComponent : public UMotionControllerComponent
{
	GENERATED_BODY()

	// Function
public:
	URepMotionControllerComponent(const FObjectInitializer& ObjectInitializer);
	void UpdateTracking(float DeltaTime);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	virtual void OnRep_ReplicatedTransform()
	{
		if (bSmoothReplicatedMotion)
		{
			if (bReppedOnce)
			{
				bLerpingPosition = true;
				NetUpdateCount = 0.0f;
				LastUpdatesRelativePosition = this->GetRelativeLocation();
				LastUpdatesRelativeRotation = this->GetRelativeRotation();
			}
			else
			{
				SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
				bReppedOnce = true;
			}
		}
		else
			SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
	}

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_SendControllerTransform(FVRSystemComponentRepPosition NewTransform);

	inline bool bHasLocalNetOwner() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();
	}

	// Variable
public:
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "RepMotionControllerComponent")
		bool bSmoothReplicatedMotion = true;
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "RepMotionControllerComponent")
		float NetUpdateFrequencyRate = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform)
	FVRSystemComponentRepPosition ReplicatedControllerTransform;

	bool bHasAuthority;
	bool bReppedOnce;
	bool bLerpingPosition;
	float NetUpdateCount = 0.0f;
	FVector LastUpdatesRelativePosition;
	FRotator LastUpdatesRelativeRotation;
};
