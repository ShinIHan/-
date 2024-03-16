// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utillity/VRSystemDataType.h"
#include "Camera/CameraComponent.h"
#include "RepCameraComponent.generated.h"

/*
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.16
 * 레퍼런스 : VRExpansionPlugin, Project_One
 */

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRSystem)
class PROJECT_TWO_API URepCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
	// Function
public:
	URepCameraComponent(const FObjectInitializer& ObjectInitializer);
	
	void UpdateTracking(float DeltaTime);
	virtual void OnAttachmentChanged() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
	
	UFUNCTION()
	virtual void OnRep_ReplicatedCameraTransform()
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
				SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
				bReppedOnce = true;
			}
		}
		else
			SetRelativeLocationAndRotation(ReplicatedCameraTransform.Position, ReplicatedCameraTransform.Rotation);
	}

	// 호출 빈도가 높으므로 Unreliable 설정
	// .cpp 에 _Implementation, _Validate
	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_SendCameraTransform(FVRSystemComponentRepPosition NewTransform);
	
	// Actor Component 가 서버에 없는지 확인
	inline bool bHasLocalNetOwner() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();
	}

	// Variable
public:
	// Component ID 복제를 피하기 위한 호출 재정의 포인터
	typedef void (ACharacter::* pCharacterRPCTransform)(FVRSystemComponentRepPosition NewTransform);
	pCharacterRPCTransform RPCTransform;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> pCharacter;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedCameraTransform, Category = "RepCameraComponent")
	FVRSystemComponentRepPosition ReplicatedCameraTransform;
	
	// Tick Logic을 돌리는 Compoenent 선택
	// True : CharacterMovement Component False : RepCameraComponent 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bUpdateInCharacterMovement;
	
	// 샘플링 설정
	UPROPERTY()
	bool bSampleVelocityInWorldSpace;
	
	// 보이지 않는 카메라의 대상위치 업데이트의 경우
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bSetPositionDuringTick;
	
	// 카메라를 현재 로컬로 제어하므로 HMD Lock 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	uint32 bAutoSetLockToHmd : 1;
	
	// 참의 경우 위치에서 HMD의 위치를 빼게 됨, HMD Location의 위치가 엑터 위치의 기반이 되는 경우 유용함.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bOffsetByHMD;
	
	// Networking Smoothing FPS 보다 낮으면 아무런 일이 일어나지 않음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "RepCameraComponent")
	bool bSmoothReplicatedMotion;
	
	// Update Frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "RepCameraComponent")
	float NetUpdateFrequencyRate;
	float NetUpdateCount;
	
	FTransform LastRelativeTransform;
	FVector LastUpdatesRelativePosition;
	FRotator LastUpdatesRelativeRotation;
	
	bool bHasAuthority;
	bool bIsServer;
	bool bLerpingPosition;
	bool bReppedOnce;
	bool bHadValidFirstVelocity;
};
