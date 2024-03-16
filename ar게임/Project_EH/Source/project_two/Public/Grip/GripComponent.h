// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grip/GripDataType.h"
#include "Components/BoxComponent.h"
#include "GripComponent.generated.h"

/**
 * 최초 작성자 : 하태웅
 * 최종 수성자 : 이용선
 * 최종 수정일 : 2023.03.22
 * 레퍼런스 : VRExpansionPlugin(GripRepMotionController), Project_One(Grip Component)
 */

/*
 * 상속된 Private Property 에 대한 Replication 변수 재정의
 * 변수가 존재하지 않을 경우 Compile-Time Error를 제거하므로 주의
 */
#define DOREPLIFETIME_ACTIVE_OVERRIDE_PRIVATE_PROPERTY(c,v,active) \
{ \
	static FProperty* sp##v = GetReplicatedProperty(StaticClass(), c::StaticClass(),FName(TEXT(#v))); \
	for (int32 i = 0; i < sp##v->ArrayDim; i++) \
	{ \
 		ChangedPropertyTracker.SetCustomIsActiveOverride(this, sp##v->RepIndex + i, active); \
	} \
}

#define RESET_REPLIFETIME_CONDITION_PRIVATE_PROPERTY(c,v,cond)  \
{\
	ResetReplicatedLifetimeProperty(StaticClass(), c::StaticClass(), FName(TEXT(#v)), cond, OutLifetimeProps); \
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGripSignature, const FGripInformation&, GripInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropSignature, const FGripInformation&, GripInformation);


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = VRSystem)
class PROJECT_TWO_API UGripComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:
	UGripComponent(const FObjectInitializer& ObjectInitializer);
	~UGripComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TickGrip(float DeltaTime);

	//Tick
	void HandleGrip(FGripInformation& HandleGripInfomation, const FTransform& ParentTransform, float DeltaTime);

	//Misc
	bool GetIsObjectHeld(const UObject* ObjectToCheck) const;
	bool GetGripWorldTransform(float DeltaTime, FTransform& WorldTransform, const FTransform& ParentTransform, FGripInformation& Grip, bool bIsForTeleport, bool& bForceADrop);
	FTransform ConvertToCompRelativeTransform(const FTransform& InTransform);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
	FOnGripSignature OnGrippedObject;
		
	UPROPERTY(BlueprintAssignable, Category = "Grip Events")
	FOnDropSignature OnDroppedObject;
	
	UPROPERTY()
	bool bHasAuthority;
		
	UPROPERTY()
	bool bWasInitiallyRepped{ false };
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	bool bIsDebug = true;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	float TraceDistance = 300;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	bool bLaserActivate = false;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	bool bDetectByOverlap = false;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	bool bIsLeft = true;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripRepMotionControllerComponent")
	bool bGripByDetectObject = true;
	
	UPROPERTY()
	UObject* TargetObj;
		
	UPROPERTY()
	TArray<UPrimitiveComponent*> HighlightedComps;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GripRepMotionControllerComponent");
	EGripDetectionType DetectionType;
	
	//Server_RepMovement 를 위한 Rep 동기화 변수
	//RPC를 통해 서버상에서 객체가 존재하는지 확인 후 -> GripInfo로 동기화하여 잡기.
	//Tick 로직이 Server 에서만 돌게 됨.
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GripRepMotionControllerComponent", ReplicatedUsing = OnRep_ServerGrippedObject)
	FGripInformation ServerGrippedObject;
		
	UFUNCTION()
	virtual void OnRep_ServerGrippedObject(FGripInformation OriginalGripInfo)
	{
		HandleGripReplication(ServerGrippedObject, &OriginalGripInfo);
	}
	
	//Owner Skip Rep ClientAuth Grip Info
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GripRepMotionControllerComponent", ReplicatedUsing = OnRep_LocalGrippedObject)
	FGripInformation LocalGrippedObject;
		
	UFUNCTION()
	virtual void OnRep_LocalGrippedObject(FGripInformation OriginalGripInfo)
	{
		HandleGripReplication(LocalGrippedObject, &OriginalGripInfo);
	}
	
	UPROPERTY()
	FGripPhysicsHandleInformation PhysicsGrip;
	
	/****************	*Detecting Func*	****************/
	UFUNCTION(Category = "GripRepMotionControllerComponent|DetectingFunction")
	void HandleDetection();
	
	UFUNCTION(Category = "GripRepMotionControllerComponent|DetectingFunction")
	UObject* DetectTargetByLaser();
		
	UFUNCTION(Category = "GripRepMotionControllerComponent|DetectingFunction")
	UObject* DetectTargetByCollision();
	UObject* GetImplementObject(UPrimitiveComponent* Comp, AActor* Actor);
	
	UFUNCTION(Category = "GripRepMotionControllerComponent|DetectingFunction")
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
		
	UFUNCTION(Category = "GripRepMotionControllerComponent|DetectingFunction")
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
		
	UFUNCTION()
	void HighlightTarget(UObject* Object);
		
	UFUNCTION()
	void ClearHighlightComps();
	
	/***********	*Grip Func* 	***********/
	UFUNCTION(BlueprintCallable, Category = "GripRepMotionControllerComponent|GripFunction")
	void Grip();
		
	UFUNCTION(BlueprintCallable, Category = "GripRepMotionControllerComponent|GripFunction")
	bool TryGrab(UObject* ObjectToGrip, const FTransform& RelativeTransform);
		
	UFUNCTION(BlueprintCallable, Category = "GripRepMotionControllerComponent|GripFunction")
	bool GrabObjectByInterface(UObject* ObjectToGrip, const FTransform& RelativeTransform);
	
	UFUNCTION(Reliable, Server, WithValidation, Category = "GripRepMotionControllerComponent|GripFunction")
	void ServerGrabObjectByInterface(UObject* ObjectToGrip, const FTransform& RelativeTransform);
		
	//Grab Event_Func
	bool Event_GripComponent(UPrimitiveComponent* TargetComp, const FTransform& RelativeTransform, EGripType GripType, EGripRepMoveType GripMovementReplicationType);
	bool Event_GripActor(AActor* TargetActor, const FTransform& RelativeTransform, EGripType GripType, EGripRepMoveType GripMovementReplicationType);
	
	//Rep Property Set 
	bool Event_SetGripInfo(const FGripInformation& GripInfo, bool bIsLocalGrip);
	
	//Grip Setup Func
	bool Event_SetupGrip(FGripInformation& GripInfomation);
	
	//For LocalGrippedObject Replication
	UFUNCTION(Reliable, Server, WithValidation, Category = "GripRepMotionControllerComponent|GripFunction")
	void ServerNotifyLocalGrip(const FGripInformation& GripInfomation);
	
	/***********	*Drop Func* 	***********/
	UFUNCTION(BlueprintCallable, Category = "GripRepMotionControllerComponent|DropFunction")
	void TryDrop();
		
	UFUNCTION(Reliable, Server, WithValidation, Category = "GripRepMotionControllerComponent|DropFunction")
	void ServerTryDrop();
	
	UFUNCTION(BlueprintCallable, Category = "GripRepMotionControllerComponent|DropFunction")
	bool DropObjectByInterface(UObject* ObjectToDrop = nullptr);
	
	//Drop Event_Func
	bool Event_DropGripInfo(const FGripInformation& Grip, bool bSimulate = false);
	void Drop_Implementation(const FGripInformation& NewDrop, bool bSimulate);
	
	UFUNCTION(Reliable, NetMulticast)
	void Multicast_NotifyDrop(const FGripInformation& NewDrop, bool bSimulate);
		
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerNotifyLocalGripRemoved(const FTransform& TransformAtDrop);
	
	/****************	*Physics Handle* 	****************/
	bool SetUpPhysicsHandle(const FGripInformation& GripInfomation);
	bool DestroyPhysicsHandle(FGripPhysicsHandleInformation& HandleInfo);
	bool DestroyPhysicsHandle(const FGripInformation& Grip);
	void OnGripMassUpdated(FBodyInstance* GripBodyInstance);
	void GetAllGrips(TArray<FGripInformation>& GripArray);
	bool UpdatePhysicsHandle(const FGripInformation& GripInfo, bool bFullyRecreate = true);
	void UpdatePhysicsHandleTransform(const FGripInformation& GrabbedActor, const FTransform& NewTransform);
	
	//Authority Func
	inline bool HasGripMovementAuthority(const FGripInformation& Grip);
	inline bool HasGripAuthority(const FGripInformation& Grip);
	
	UFUNCTION(BlueprintPure, Category = "GripRepMotionControllerComponent", meta = (DisplayName = "HasGripAuthority"))
	bool BP_HasGripAuthority(const FGripInformation& Grip);
	
	//Network Grip
	inline bool HandleGripReplication(FGripInformation& Grip, FGripInformation* OldGripInfo = nullptr)
	{
		if (!Grip.TargetObject)
			return false;
		// Ignore server down no rep grips, this is kind of unavoidable unless I make yet another list which I don't want to do
		if (Grip.MovementReplicationType == EGripRepMoveType::NoRepMove)
		{
			// skip init
			bWasInitiallyRepped = true;
	
			// null ptr so this doesn't block grip operations
			Grip.TargetObject = nullptr;
		}
	
		if (!bWasInitiallyRepped) // Hasn't already been initialized
		{
			bWasInitiallyRepped = Event_SetupGrip(Grip); // Grip it
				
			// Tick will keep checking from here on out locally
			if (!bWasInitiallyRepped)
				return false;
		}
		return true;
	}
	
	inline bool IsServer() const
	{
		if (GEngine != nullptr && GWorld != nullptr)
			return GEngine->GetNetMode(GWorld) < NM_Client;
	
		return false;
	}
	
	inline bool bHasLocalNetOwner() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner->HasLocalNetOwner();
	}
	
	inline bool IsTornOff() const
	{
		const AActor* MyOwner = GetOwner();
		return MyOwner ? MyOwner->GetTearOff() : false;
	}
	
	FORCEINLINE FTransform GetPivotTransform()
	{
		return this->GetComponentTransform();
	}
};
	
bool inline UGripComponent::HasGripMovementAuthority(const FGripInformation& Grip)
{
	if (IsServer())
	{
		return true;
	}
	else
	{
		if (Grip.MovementReplicationType == EGripRepMoveType::Client_CompAttach 
			|| Grip.MovementReplicationType == EGripRepMoveType::NoRepMove)
		{
			return true;
		}
		else if (Grip.MovementReplicationType == EGripRepMoveType::Server_RepMovement)
		{
			return false;
		}
	}
	return false;
}
	
bool inline UGripComponent::HasGripAuthority(const FGripInformation& Grip)
{
	if (   ((Grip.MovementReplicationType != EGripRepMoveType::Client_CompAttach && Grip.MovementReplicationType != EGripRepMoveType::NoRepMove) && IsServer())
		|| ((Grip.MovementReplicationType == EGripRepMoveType::Client_CompAttach || Grip.MovementReplicationType == EGripRepMoveType::NoRepMove) && bHasAuthority))
		return true;
	return false;
}

inline bool UGripComponent::BP_HasGripAuthority(const FGripInformation& Grip)
{
	return HasGripAuthority(Grip);
}
