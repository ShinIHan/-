// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/ConstraintDrives.h"
#include "GripDataType.generated.h"

/*
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.21
 * 레퍼런스 : VRExpansionPlugin, Project_One
 *
*/

#define INVALID_GRIP_ID 0

/*****			Grip Enums			*****/
UENUM(BlueprintType)
enum class EGripDetectionType : uint8
{
	Collision,
	Laser,
	Both,//Collision 우선
};

UENUM(BlueprintType)
enum class EGripTargetType : uint8
{
	ActorGrip,
	ComponentGrip
};

UENUM(Blueprintable)
enum class EGripType : uint8
{
	PhysicsGrip,
	NoPhysicsGrip,
	PhysicsSocketGrip,
	NoPhysicsSocketGrip,
	CustomGrip,
	EventGrip
};

UENUM(Blueprintable)
enum class EGripRepMoveType : uint8
{
	Server_RepMovement,
	Client_CompAttach,
	NoRepMove
};


/*****			Grip Rep Info			*****/
USTRUCT(BlueprintType, Category = "GripDataType")
struct PROJECT_TWO_API FGripInformation
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "GripDataType|GripRepInfomation")
	UObject* TargetObject;
	
	UPROPERTY(BlueprintReadOnly, Category = "GripDataType|GripRepInfomation")
	EGripTargetType TargetType;

	UPROPERTY(BlueprintReadOnly, Category = "GripDataType|GripRepInfomation")
	EGripType Type;
	
	UPROPERTY(BlueprintReadOnly, Category = "GripDataType|GripRepInfomation")
	EGripRepMoveType MovementReplicationType;
	
	UPROPERTY(BlueprintReadWrite, Category = "GripDataType|GripRepInfomation")
	FTransform RelativeTransform;

	//Original Settings
	UPROPERTY()
	bool bOriginalRepMov;

	UPROPERTY()
	bool bOriginalGravity;

	FGripInformation() :
		TargetObject(nullptr),
		TargetType(EGripTargetType::ActorGrip),
		Type(EGripType::PhysicsGrip),
		MovementReplicationType(EGripRepMoveType::NoRepMove),
		RelativeTransform(FTransform::Identity),
		bOriginalRepMov(false),
		bOriginalGravity(false)
	{
	}

	void Reset()
	{
		TargetType = EGripTargetType::ActorGrip;
		TargetObject = nullptr;
		Type = EGripType::PhysicsGrip;
		RelativeTransform = FTransform::Identity;
		MovementReplicationType = EGripRepMoveType::NoRepMove;
		bOriginalRepMov = false;
		bOriginalGravity = false;
	}

	FORCEINLINE FGripInformation& RepCopy(const FGripInformation& Other)
	{
		this->TargetType = Other.TargetType;
		this->TargetObject = Other.TargetObject;
		this->Type = Other.Type;
		this->RelativeTransform = Other.RelativeTransform;
		this->MovementReplicationType = Other.MovementReplicationType;
		this->bOriginalRepMov = Other.bOriginalRepMov;
		this->bOriginalGravity = Other.bOriginalGravity;
		return *this;
	}

	FORCEINLINE AActor* GetGrabbedActor() const
	{
		return Cast<AActor>(TargetObject);
	}

	FORCEINLINE UPrimitiveComponent* GetGrabbedComponent() const
	{
		return Cast<UPrimitiveComponent>(TargetObject);
	}

	FORCEINLINE bool operator==(const FGripInformation& Other) const
	{
		if (TargetObject && TargetObject == Other.TargetObject) return true;
		return false;
	}

	FORCEINLINE bool operator==(const AActor* Other) const
	{
		if (Other && TargetObject && TargetObject == (const UObject*)Other) return true;
		return false;
	}

	FORCEINLINE bool operator==(const UPrimitiveComponent* Other) const
	{
		if (Other && TargetObject && TargetObject == (const UObject*)Other) return true;
		return false;
	}

	FORCEINLINE bool operator==(const UObject* Other) const
	{
		if (Other && TargetObject == Other) return true;
		return false;
	}

	bool IsValid()
	{
		return TargetObject != nullptr;
	}
};

/*		Grip Setting		*/
USTRUCT(BlueprintType, Category = "GripDataType")
struct PROJECT_TWO_API FGripSetting
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripDataType|GripSetting")
	EGripType Type;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripDataType|GripSetting")
	EGripRepMoveType MovementReplicationType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripDataType|GripSetting")
	bool bDropSimulate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripDataType|GripSetting")
	bool bDenyGripping;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripDataType|GripSetting")
	bool bDenyHighlighting;
	
	FGripSetting() :
		Type(EGripType::PhysicsGrip),
		MovementReplicationType(EGripRepMoveType::NoRepMove),
		bDropSimulate(true), bDenyGripping(false), bDenyHighlighting(true)
	{
	}
};

USTRUCT(BlueprintType, Category = "GripDataType")
struct PROJECT_TWO_API FGripPhysicsHandleInformation
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "GripDataType|GripPhysicsInformation")
	UObject* HandledObject;

	FPhysicsActorHandle KinActorData;
	FPhysicsConstraintHandle HandleData;
	FLinearDriveConstraint LinearConstraint;
	FAngularDriveConstraint AngularConstraint;

	FTransform LastPhysicsTransform;
	FTransform RootBoneRotation;

	FGripPhysicsHandleInformation()
	{
		HandledObject = nullptr;
		LastPhysicsTransform = FTransform::Identity;
		RootBoneRotation = FTransform::Identity;
		KinActorData = nullptr;
	}

	void Reset()
	{
		HandledObject = nullptr;
		LastPhysicsTransform = FTransform::Identity;
		RootBoneRotation = FTransform::Identity;
		KinActorData = nullptr;
	}

	FORCEINLINE bool operator==(const FGripInformation& Other) const
	{
		return (HandledObject == Other.TargetObject);
	}
};