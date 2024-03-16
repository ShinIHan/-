// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RepCameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "IXRCamera.h"
#include "IHeadMountedDisplay.h"
#include "Rendering/MotionVectorSimulation.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

//bool TMP_IsHeadTrackingAllowedForWorld(IXRTrackingSystem* XRSystem, UWorld* World)
//{
//#if WITH_EDITOR
//	// This implementation is constrained by hotfix rules.  It would be better to cache this somewhere.
//	if (!XRSystem->IsHeadTrackingAllowed())
//		return false;
//
//	if (World->WorldType != EWorldType::PIE)
//		return true;
//
//	// PIE 인스턴스라면 Dedicated Server가 아닌 첫번째 PIE World는 해드 트래킹을 사용합니다.
//	const int32 MyPIEInstanceID = World->GetOutermost()->GetPIEInstanceID();
//	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
//	{
//		if (WorldContext.WorldType == EWorldType::PIE && WorldContext.RunAsDedicated == false && WorldContext.World())
//			return WorldContext.World()->GetOutermost()->GetPIEInstanceID() == MyPIEInstanceID;
//	}
//
//	return false;
//#endif
//
//	return XRSystem->IsHeadTrackingAllowedForWorld(*World);
//}

// UProperty Replicated 설정시
void URepCameraComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//기존 Scene Comp Rep 변수 해제	
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	// 현재 Component의 Replication Props
	DOREPLIFETIME_CONDITION(URepCameraComponent, ReplicatedCameraTransform, COND_SkipOwner);
	DOREPLIFETIME(URepCameraComponent, NetUpdateFrequencyRate);
	DOREPLIFETIME(URepCameraComponent, bSmoothReplicatedMotion);
}

URepCameraComponent::URepCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	//UActorComponenet
	SetIsReplicatedByDefault(true);

	//SceneComponenet
	SetRelativeScale3D(FVector(1.0f));

	// Net Update Frequency
	// Rep_notify의 업데이트 속도 100과 동일
	// HMD의 Vsync로 인해 90 or 45로 제한될 수 있음
	NetUpdateFrequencyRate = 100.0f;
	NetUpdateCount = 0.0f;
	
	bSmoothReplicatedMotion = false;
	
	bUsePawnControlRotation = false;
	bAutoSetLockToHmd = true;
	bOffsetByHMD = false;
	
	bSetPositionDuringTick = false;
	bLerpingPosition = false;
	bReppedOnce = false;
	
	RPCTransform = nullptr;
	
	LastRelativeTransform = FTransform::Identity;
	bSampleVelocityInWorldSpace = false;
	bHadValidFirstVelocity = false;
}

void URepCameraComponent::UpdateTracking(float DeltaTime)
{
	bHasAuthority = bHasLocalNetOwner();

	// Authority가 있는 경우
	if (bHasAuthority)
	{
		if (bSetPositionDuringTick && 
			bLockToHmd && 
			GEngine->XRSystem.IsValid() && 
			GEngine->XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
		{
			FQuat Orientation;
			FVector Position;
			if (GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, Orientation, Position))
			{
				if (bOffsetByHMD)
				{
					Position.X = 0;
					Position.Y = 0;
				}

				SetRelativeTransform(FTransform(Orientation, Position));
			}
		}
	}
	// Authority가 없는 경우
	else
	{
		if (bLerpingPosition)
		{
			NetUpdateCount += DeltaTime;
			float LerpVal = FMath::Clamp(NetUpdateCount / (1.0f / NetUpdateFrequencyRate), 0.0f, 1.0f);

			if (LerpVal >= 1.0f)
			{
				SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);

				// Lerping을 중지하고 다음 업데이트가 지연되거나 손실된 경우 다음 업데이트를 기다리면 문제가 생긴다.
				// 예측 기능은 미래에 고려해야 할 사항일 수 있으나 움직임의 속도와 정확성을 고려하면 VR에서 진행하기 어렵다.
				bLerpingPosition = false;
				NetUpdateCount = 0.0f;
			}
			else
			{
				SetRelativeLocationAndRotation(
					FMath::Lerp(LastUpdatesRelativePosition, (FVector)ReplicatedCameraTransform.Position, LerpVal),
					FMath::Lerp(LastUpdatesRelativeRotation, ReplicatedCameraTransform.Rotation, LerpVal)
				);
			}
		}
	}

	// Last Frame의 Velocity 저장
	if (bHadValidFirstVelocity || !LastRelativeTransform.Equals(FTransform::Identity))
	{
		bHadValidFirstVelocity = true;
		ComponentVelocity = ((bSampleVelocityInWorldSpace ? GetComponentLocation() : GetRelativeLocation()) - LastRelativeTransform.GetTranslation()) / DeltaTime;
	}

	LastRelativeTransform = bSampleVelocityInWorldSpace ? this->GetComponentTransform() : this->GetRelativeTransform();
}

void URepCameraComponent::OnAttachmentChanged()
{
	if (ACharacter* pOwner = Cast<ACharacter>(this->GetOwner()))
	{
		pCharacter = pOwner;
	}
	else
	{
		pCharacter.Reset();
	}

	Super::OnAttachmentChanged();
}

void URepCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUpdateInCharacterMovement || !pCharacter.IsValid())
	{
		UpdateTracking(DeltaTime);
	}
	else
	{
		UCharacterMovementComponent* CharMove = pCharacter->GetCharacterMovement();
		if (!CharMove ||
			!CharMove->IsComponentTickEnabled() ||
			!CharMove->IsActive() ||
			(!CharMove->PrimaryComponentTick.bTickEvenWhenPaused && GetWorld()->IsPaused()))
		{
			UpdateTracking(DeltaTime);
		}
	}

	if (bHasAuthority)
	{
		// Send Changes
		if (this->GetIsReplicated())
		{
			FVector RelativeLoc = GetRelativeLocation();
			FRotator RelativeRot = GetRelativeRotation();

			// 변경사항이 없다면 Replication 하지 않음
			if (!RelativeLoc.Equals(ReplicatedCameraTransform.Position) ||
				!RelativeRot.Equals(ReplicatedCameraTransform.Rotation))
			{
				NetUpdateCount += DeltaTime;

				if (NetUpdateCount >= (1.0f / NetUpdateFrequencyRate))
				{
					NetUpdateCount = 0.0f;
					ReplicatedCameraTransform.Position = RelativeLoc;
					ReplicatedCameraTransform.Rotation = RelativeRot;

					// NM : Net Mode
					// NM_Client : Network Client : 원격 서버에 연결된 클라이언트
					// 이 값보다 작은 모든 모드는 일종의 서버이다.
					if (GetNetMode() == NM_Client)
					{
						ACharacter* OwningChar = Cast<ACharacter>(GetOwner());
						if (RPCTransform != nullptr && OwningChar != nullptr)
						{
							(OwningChar->*(RPCTransform))(ReplicatedCameraTransform);
						}
						else
						{
							Server_SendCameraTransform(ReplicatedCameraTransform);
						}
					}
				}
			}
		}
	}
}

void URepCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	bool bIsLocallyControlled = bHasLocalNetOwner();

	if (bAutoSetLockToHmd)
	{
		if (bIsLocallyControlled)
			bLockToHmd = true;
		else
			bLockToHmd = false;
	}

	if (bIsLocallyControlled 
		&& GEngine 
		&& GEngine->XRSystem.IsValid() 
		&& GetWorld() 
		&& GetWorld()->WorldType != EWorldType::Editor)
	{
		IXRTrackingSystem* XRSystem = GEngine->XRSystem.Get();
		auto XRCamera = XRSystem->GetXRCamera();

		if (XRCamera.IsValid())
		{
			//if (TMP_IsHeadTrackingAllowedForWorld(XRSystem, GetWorld()))
			if (XRSystem->IsHeadTrackingAllowedForWorld(*GetWorld()))
			{
				const FTransform ParentWorld = CalcNewComponentToWorld(FTransform());
				XRCamera->SetupLateUpdate(ParentWorld, this, bLockToHmd == 0);

				if (bLockToHmd)
				{
					FQuat Orientation;
					FVector Position;
					if (XRCamera->UpdatePlayerCamera(Orientation, Position))
					{
						if (bOffsetByHMD)
						{
							Position.X = 0;
							Position.Y = 0;
						}

						SetRelativeTransform(FTransform(Orientation, Position));
					}
					else
					{
						SetRelativeScale3D(FVector(1.0f));
					}
				}

				XRCamera->OverrideFOV(this->FieldOfView);
			}
		}
	}

	if (bUsePawnControlRotation)
	{
		const APawn* OwningPawn = Cast<APawn>(GetOwner());
		const AController* OwningController = OwningPawn ? OwningPawn->GetController() : nullptr;
		if (OwningController && OwningController->IsLocalPlayerController())
		{
			const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
			if (!PawnViewRotation.Equals(GetComponentRotation()))
			{
				SetWorldRotation(PawnViewRotation);
			}
		}
	}

	if (bUseAdditiveOffset)
	{
		FTransform OffsetCamToBaseCam = AdditiveOffset;
		FTransform BaseCamToWorld = GetComponentToWorld();
		FTransform OffsetCamToWorld = OffsetCamToBaseCam * BaseCamToWorld;

		DesiredView.Location = OffsetCamToWorld.GetLocation();
		DesiredView.Rotation = OffsetCamToWorld.Rotator();
	}
	else
	{
		DesiredView.Location = GetComponentLocation();
		DesiredView.Rotation = GetComponentRotation();
	}

	DesiredView.FOV = bUseAdditiveOffset ? (FieldOfView + AdditiveFOVOffset) : FieldOfView;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;

	// CameraActor에 사용된 PostProcess 설정을 덮어 쓰고 싶은지 확인
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}

	// Camera Component에 Motion Vector Simulation Transform이 있는 경우 use that for the current view's previous transform
	DesiredView.PreviousViewTransform = FMotionVectorSimulation::Get().GetPreviousTransform(this);
}

// ***************************
// Server_SendCameraTransform
void URepCameraComponent::Server_SendCameraTransform_Implementation(FVRSystemComponentRepPosition NewTransform)
{
	// OnRep_Function 실행시 NewTransform 저장
	ReplicatedCameraTransform = NewTransform;

	// 서버가 이 컨트롤러를 제어하는 경우 서버에서 OnRep을 호출하지 마십시오.
	if (!bHasAuthority)
	{
		OnRep_ReplicatedCameraTransform();
	}
}

bool URepCameraComponent::Server_SendCameraTransform_Validate(FVRSystemComponentRepPosition NewTransform)
{
	return true;
}

