// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utillity/VRSystemDataType.h"
#include "Camera/CameraComponent.h"
#include "RepCameraComponent.generated.h"

/*
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.16
 * ���۷��� : VRExpansionPlugin, Project_One
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

	// ȣ�� �󵵰� �����Ƿ� Unreliable ����
	// .cpp �� _Implementation, _Validate
	UFUNCTION(Unreliable, Server, WithValidation)
	void Server_SendCameraTransform(FVRSystemComponentRepPosition NewTransform);
	
	// Actor Component �� ������ ������ Ȯ��
	inline bool bHasLocalNetOwner() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();
	}

	// Variable
public:
	// Component ID ������ ���ϱ� ���� ȣ�� ������ ������
	typedef void (ACharacter::* pCharacterRPCTransform)(FVRSystemComponentRepPosition NewTransform);
	pCharacterRPCTransform RPCTransform;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> pCharacter;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ReplicatedCameraTransform, Category = "RepCameraComponent")
	FVRSystemComponentRepPosition ReplicatedCameraTransform;
	
	// Tick Logic�� ������ Compoenent ����
	// True : CharacterMovement Component False : RepCameraComponent 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bUpdateInCharacterMovement;
	
	// ���ø� ����
	UPROPERTY()
	bool bSampleVelocityInWorldSpace;
	
	// ������ �ʴ� ī�޶��� �����ġ ������Ʈ�� ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bSetPositionDuringTick;
	
	// ī�޶� ���� ���÷� �����ϹǷ� HMD Lock 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	uint32 bAutoSetLockToHmd : 1;
	
	// ���� ��� ��ġ���� HMD�� ��ġ�� ���� ��, HMD Location�� ��ġ�� ���� ��ġ�� ����� �Ǵ� ��� ������.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RepCameraComponent")
	bool bOffsetByHMD;
	
	// Networking Smoothing FPS ���� ������ �ƹ��� ���� �Ͼ�� ����
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
