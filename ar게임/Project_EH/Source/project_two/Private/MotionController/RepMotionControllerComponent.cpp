// Fill out your copyright notice in the Description page of Project Settings.


#include "MotionController/RepMotionControllerComponent.h"
#include "Net/UnrealNetwork.h"

// UProperty Replicated
void URepMotionControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 기존 변수 Replicaion 해제
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	// 현재 Compoenet의 Replication Props
	DOREPLIFETIME_CONDITION(URepMotionControllerComponent, ReplicatedControllerTransform, COND_SkipOwner);
	DOREPLIFETIME(URepMotionControllerComponent, NetUpdateFrequencyRate);
	DOREPLIFETIME(URepMotionControllerComponent, bSmoothReplicatedMotion);
}

URepMotionControllerComponent::URepMotionControllerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bHasAuthority = false;
	bAutoActivate = true;
	bReppedOnce = false;
	bLerpingPosition = false;
}

void URepMotionControllerComponent::UpdateTracking(float DeltaTime)
{
	bHasAuthority = bHasLocalNetOwner();

	if (bHasAuthority)
	{
		//권한이 있으면 자기 현재 Position을 Rep
		if (GetIsReplicated())
		{
			const FVector RelLoc = GetRelativeLocation();
			const FRotator RelRot = GetRelativeRotation();

			// 저장 변수와 값이 다르다면 업데이트.
			if (!RelLoc.Equals(ReplicatedControllerTransform.Position) || !RelRot.Equals(ReplicatedControllerTransform.Rotation))
			{
				NetUpdateCount += DeltaTime;
				if (NetUpdateCount >= (1.0f / NetUpdateFrequencyRate))
				{
					NetUpdateCount = 0.0f;

					ReplicatedControllerTransform.Position = RelLoc;
					ReplicatedControllerTransform.Rotation = RelRot;

					if (GetNetMode() == NM_Client) //소유권 있는 클라이언트라면 전송.
					{
						Server_SendControllerTransform(ReplicatedControllerTransform);
					}
				}
			}
		}
	}
	//소유 권한이 없으면 값을 받아와서 동기화
	else
	{
		if (bLerpingPosition)
		{
			NetUpdateCount += DeltaTime;
			float LerpVal = FMath::Clamp(NetUpdateCount / (1.0f / NetUpdateFrequencyRate), 0.0f, 1.0f);

			if (LerpVal >= 1.0f)
			{
				SetRelativeLocationAndRotation(ReplicatedControllerTransform.Position, ReplicatedControllerTransform.Rotation);
				bLerpingPosition = false;
				NetUpdateCount = 0.0f;
			}
			else
			{
				SetRelativeLocationAndRotation(
					FMath::Lerp(LastUpdatesRelativePosition, (FVector)ReplicatedControllerTransform.Position, LerpVal),
					FMath::Lerp(LastUpdatesRelativeRotation, ReplicatedControllerTransform.Rotation, LerpVal)
				);
			}
		}
	}
}

void URepMotionControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTracking(DeltaTime);
}

// ***************************
// Server_SendControllerTransform
void URepMotionControllerComponent::Server_SendControllerTransform_Implementation(FVRSystemComponentRepPosition NewTransform)
{
	ReplicatedControllerTransform = NewTransform;
	if (!bHasAuthority)
	{
		OnRep_ReplicatedTransform();
	}
}

bool URepMotionControllerComponent::Server_SendControllerTransform_Validate(FVRSystemComponentRepPosition NewTransform)
{
	return true;
}
