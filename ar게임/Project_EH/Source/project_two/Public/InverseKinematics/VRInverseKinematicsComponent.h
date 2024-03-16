// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRInverseKinematicsComponent.generated.h"

/*
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.17
 * 레퍼런스 : VRExpansionPlugin, Project_One
 * 
 * 사용 용어
 * Effector : HMD, Motion Controller와 같이 영향을 주는 오브젝트
 * 
 */

// Atmomic : 에디터의 디테일 창에서 표시되고 수정 가능하도록 지정
// BlueprintType : 디테일 창에서는 보이지 않고 코드 내부나 블루프린트에서만 사용되기를 원하는 경우 사용
USTRUCT(BlueprintType)
struct PROJECT_TWO_API FUpperBodyIKData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator Head;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator Pelvis;

	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator LeftClavicle;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator LeftUpperArm;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator LeftLowerArm;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator LeftHand;

	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator RightClavicle;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator RightUpperArm;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator RightLowerArm;
	UPROPERTY(BlueprintReadOnly, Category = "VRInverseKinematicsComponent")
	FRotator RightHand;
};

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = VRSystem)
class PROJECT_TWO_API UVRInverseKinematicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVRInverseKinematicsComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitialzieBoneLenghtToSkeletalMesh(USkeletalMeshComponent* SkeletalMeshComp);

	UFUNCTION(BlueprintCallable)
	void SetEffectorTransforms(const FTransform& Origin, const FTransform& Head, const FTransform& Left, const FTransform& Right);

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerUpdateIKData(FUpperBodyIKData ClientIKData);

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	FUpperBodyIKData UpperBodyIKData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRInverseKinematicsComponent")
	bool bDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRInverseKinematicsComponent")
	FVector HandOffset = FVector(9.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRInverseKinematicsComponent")
	FRotator HandRotationOffset = FRotator(0.0f, 0.0f, 0.0f);

	//Skeletal Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRInverseKinematicsComponent")
	float Height = 185.f;

	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	FVector NeckOffset = FVector(-9.f, 0.f, -7.f);

	//양 쪽 팔 사이 거리 Upper끼리의 Dist
	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	float UpperArmsDistance = 31.f;

	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	float ArmLength;

	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	float LowerArmLength;

	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	float UpperArmLength;

	UPROPERTY(VisibleInstanceOnly, Category = "VRInverseKinematicsComponent")
	float ClavicleOffset = -20.f;

	/* Settings But Use Default.*/
	float HeadHandAngleLimit = 150.f;
	float HeadHandAngleLimitDot;
	float DistinctShoulderRotationLimit = 45.f;
	float DistinctShoulderRotationMultiplier = 60.f;
	float ElbowBaseOffsetAngle = 130.f;
	float ElbowHandsRotSpeed = 15.f;
	float ElbowRotFromHandRotAlpha = 0.6f;
	float ElbowYDistanceStart = 0.2f;
	float ElbowYWeight = 130.f;
	float OkSpanAngle = 80.f;
	float ShoulderHeadHandAlpha = 0.75f;

	// World
	FTransform Head_World;
	FTransform LeftHand_World;
	FTransform RightHand_World;
	UPROPERTY(BlueprintReadOnly)
	FTransform Shoulder_World;
		//Left
	FTransform LeftUpperArm_World; 
	FTransform LeftLowerArm_World;
		//Right
	FTransform RightUpperArm_World; 
	FTransform RightLowerArm_World;

// Function
private:
	void ConvertTransforms();
	void SetShoulder();
	void SetUpperArms();
	void SolveArms();

	//ConvertTransformsFunc
	FTransform AddLocalOffset(const FTransform& Transform, const FVector Offset);

	//SetShoulderFunc
	FRotator GetShoulderRotationFromHands();
	float GetHeadHandAngle(float LastAngle, const FVector& Hand, const FVector& HandHeadDelta) const;
	FVector GetShoulderLocation();
	FTransform RotatePointAroundPivot(FTransform Point, FTransform Pivot, FRotator Delta);

	//SetUpperArm Func
	FTransform RotateUpperArm(bool isLeft, const FVector& HandTranslation);

	//SolveArms Func
	void SetElbowBasePosition(const FVector& UpperArm, const FVector& Hand, bool IsLeft, FTransform& UpperArmTransform, FTransform& LowerArmTransform);
	void RotateElbow(float Angle, const FTransform& UpperArm, const FTransform& LowerArm, const FVector& HandLoc, bool IsLeft, FTransform& NewUpperArm, FTransform& NewLowerArm);
	float RotateElbowByHandPosition(const FVector& Hand, bool IsLeft);
	float RotateElbowByHandRotation(const FTransform& LowerArm, FRotator Hand);
	float CosineRule(float Adjacent1, float Adjacent2, float Opposite);
	float SafeguardAngle(float Last, float Current, float Threshold);

	//Update IK Data
	void UpdateIKData();
	void DrawDebug();

	inline bool bHasLocalNetOwner() const
	{
		const AActor* Owner = GetOwner();
		return Owner->HasLocalNetOwner();
	}

// Variables
private:
	//Input
	FTransform Origin_World;
	FTransform HeadEffector_World;
	FTransform LeftEffector_World;
	FTransform RightEffector_World;

	//Local
	FTransform Origin_Local;
	FTransform Head_Local;
	FTransform LeftHand_Local;
	FTransform RightHand_Local;
	FTransform Shoulder_Local;
		//Left
	FTransform LeftClavicle_Local;
	FTransform LeftUpperArm_Local;
	FTransform LeftLowerArm_Local;
		//Right
	FTransform RightClavicle_Local;
	FTransform RightUpperArm_Local;
	FTransform RightLowerArm_Local;

	//Shoulder
	FTransform Head_Shoulder;
	FTransform LeftHand_Shoulder;
	FTransform RightHand_Shoulder;
	FTransform Shoulder_Shoulder;
		//Left
	FTransform LeftUpperArm_Shoulder;
	FTransform LeftLowerArm_Shoulder;
		//Right
	FTransform RightUpperArm_Shoulder;
	FTransform RightLowerArm_Shoulder;

	float CachedDeltaTime;

	float LeftHeadHandAngle;
	float RightHeadHandAngle;
	float LeftElbowHandAngle;
	float RightElbowHandAngle;
};
