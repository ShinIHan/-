// Fill out your copyright notice in the Description page of Project Settings.


#include "MotionController/RepMotionControllerComponent.h"
#include "Net/UnrealNetwork.h"

// UProperty Replicated
void URepMotionControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ���� ���� Replicaion ����
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	// ���� Compoenet�� Replication Props
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
		//������ ������ �ڱ� ���� Position�� Rep
		if (GetIsReplicated())
		{
			const FVector RelLoc = GetRelativeLocation();
			const FRotator RelRot = GetRelativeRotation();

			// ���� ������ ���� �ٸ��ٸ� ������Ʈ.
			if (!RelLoc.Equals(ReplicatedControllerTransform.Position) || !RelRot.Equals(ReplicatedControllerTransform.Rotation))
			{
				NetUpdateCount += DeltaTime;
				if (NetUpdateCount >= (1.0f / NetUpdateFrequencyRate))
				{
					NetUpdateCount = 0.0f;

					ReplicatedControllerTransform.Position = RelLoc;
					ReplicatedControllerTransform.Rotation = RelRot;

					if (GetNetMode() == NM_Client) //������ �ִ� Ŭ���̾�Ʈ��� ����.
					{
						Server_SendControllerTransform(ReplicatedControllerTransform);
					}
				}
			}
		}
	}
	//���� ������ ������ ���� �޾ƿͼ� ����ȭ
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
