// Fill out your copyright notice in the Description page of Project Settings.


#include "Grip/GripComponent.h"

#include "Grip/GripInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsReplication.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Physics/Experimental/PhysScene_Chaos.h"

#include "PhysicsPublic.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConstraintDrives.h"
#include "PhysicsReplication.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "Chaos/ParticleHandle.h"
#include "Chaos/KinematicGeometryParticles.h"
#include "Chaos/PBDJointConstraintTypes.h"
#include "Chaos/PBDConstraintBaseData.h"
#include "Chaos/PBDJointConstraintData.h"
#include "Chaos/Sphere.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Chaos/ChaosConstraintSettings.h"

const float DEFAULT_STIFFNESS = 1500.f;
const float DEFAULT_DAMPING = 200.f;
const float ANGULAR_STIFFNESS_MULTIPLIER = 1.5f;
const float ANGULAR_DAMPING_MULTIPLIER = 1.4f;

void UGripComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Transform 동기화의 이유가 없음.
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	//Grip Obj Rep
	DOREPLIFETIME(UGripComponent, ServerGrippedObject);
	DOREPLIFETIME_CONDITION(UGripComponent, LocalGrippedObject, COND_SkipOwner);
}

UGripComponent::UGripComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	bHasAuthority = false;
	bAutoActivate = true;

	SetIsReplicatedByDefault(true);
}

UGripComponent::~UGripComponent()
{
}

void UGripComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	bHasAuthority = bHasLocalNetOwner();
	if (!ServerGrippedObject.IsValid() && !LocalGrippedObject.IsValid() && bHasAuthority)
	{
		if (DetectionType != EGripDetectionType::Collision && bLaserActivate)
		{
			HandleDetection();
		}
	}

	TickGrip(DeltaTime);
}

void UGripComponent::TickGrip(float DeltaTime)
{
	FTransform ParentTransform = GetPivotTransform();
	HandleGrip(ServerGrippedObject, ParentTransform, DeltaTime);
	HandleGrip(LocalGrippedObject, ParentTransform, DeltaTime);
}

void UGripComponent::HandleGrip(FGripInformation& HandleGripInfomation, const FTransform& ParentTransform, float DeltaTime)
{
	if (HandleGripInfomation.IsValid() && IsValid(HandleGripInfomation.TargetObject))
	{

		if (!HasGripMovementAuthority(HandleGripInfomation))
			return;
		if (!bWasInitiallyRepped && HasGripAuthority(HandleGripInfomation) && !HandleGripReplication(HandleGripInfomation))
			return;
		if (HandleGripInfomation.Type == EGripType::EventGrip)
			return;

		UPrimitiveComponent* root = NULL;
		AActor* actor = NULL;

		//Set Root, Target Actor
		switch (HandleGripInfomation.TargetType)
		{
		case EGripTargetType::ActorGrip:
		{
			actor = HandleGripInfomation.GetGrabbedActor();
			if (actor)
				root = Cast<UPrimitiveComponent>(actor->GetRootComponent());
		}break;

		case EGripTargetType::ComponentGrip:
		{
			root = HandleGripInfomation.GetGrabbedComponent();
			if (root)
				actor = root->GetOwner();
		}break;

		default:
			break;
		}

		if (!root || !actor)
			return;

		bool bRootHasInterface = false;
		bool bActorHasInterface = false;

		if (root->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			bRootHasInterface = true;
		}
		else if (actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			bActorHasInterface = true;
		}

		if (HandleGripInfomation.Type == EGripType::CustomGrip)
		{
			if (bRootHasInterface)
			{
				IGripInterface::Execute_TickGrip(root, this, HandleGripInfomation, DeltaTime);
			}
			else if (bActorHasInterface)
			{
				IGripInterface::Execute_TickGrip(actor, this, HandleGripInfomation, DeltaTime);
			}
			return;
		}

		bool bForceADrop = false;
		FTransform WorldTransform = HandleGripInfomation.RelativeTransform * ParentTransform;

		switch (HandleGripInfomation.Type)
		{
		case EGripType::PhysicsGrip:
		{
			UpdatePhysicsHandleTransform(HandleGripInfomation, WorldTransform);
			root->SetWorldScale3D(WorldTransform.GetScale3D());
		}
		break;
		case EGripType::NoPhysicsGrip:
		{
			FTransform RelativeTrans = WorldTransform.GetRelativeTransform(ParentTransform);
			if (!root->GetAttachParent() || root->IsSimulatingPhysics() || !HandleGripInfomation.RelativeTransform.Equals(RelativeTrans, 1.f))
			{
				root->SetWorldTransform(HandleGripInfomation.RelativeTransform * this->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);
				root->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,	EAttachmentRule::KeepWorld, true));
			}
		}break;

		default:
			break;
		}
	}
}

bool UGripComponent::GetIsObjectHeld(const UObject* ObjectToCheck) const
{
	if (!ObjectToCheck)	return false;
	return (ServerGrippedObject.TargetObject == ObjectToCheck || LocalGrippedObject.TargetObject == ObjectToCheck);
}

bool UGripComponent::GetGripWorldTransform(float DeltaTime, FTransform& WorldTransform, const FTransform& ParentTransform, FGripInformation& Grip, bool bIsForTeleport, bool& bForceADrop)
{
	bool bHasValidTransform = true;
	WorldTransform = Grip.RelativeTransform /* * Grip.AdditionTransform */ * ParentTransform;
	bForceADrop = false;

	return bHasValidTransform;
}

FTransform UGripComponent::ConvertToCompRelativeTransform(const FTransform& InTransform)
{
	return InTransform.GetRelativeTransform(this->GetComponentTransform());
}

void UGripComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (DetectionType != EGripDetectionType::Laser && bHasLocalNetOwner())
	{
		OnComponentBeginOverlap.AddDynamic(this, &UGripComponent::OnBeginOverlap);
		OnComponentEndOverlap.AddDynamic(this, &UGripComponent::OnEndOverlap);
	}
}

void UGripComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnGrippedObject.Clear();
	OnDroppedObject.Clear();
}

/********************************************************/
/****************	*Detecting Function*	****************/
/********************************************************/
void UGripComponent::HandleDetection()
{
	if (UObject* Obj = DetectTargetByLaser())
	{
		if (TargetObj == Obj)
			return;

		TargetObj = Obj;
		ClearHighlightComps();
		HighlightTarget(Obj);
	}
	else
	{
		if (TargetObj)
		{
			ClearHighlightComps();
			TargetObj = nullptr;
		}
	}
}

UObject* UGripComponent::DetectTargetByLaser()
{
	FHitResult HitResult;
	const FVector Start = GetComponentLocation();
	const FVector End = Start + (GetForwardVector() * TraceDistance);
	const TArray<AActor*> ActorToIgnore{ GetOwner() };
	if (UKismetSystemLibrary::LineTraceSingle(this, Start, End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true, ActorToIgnore,
		bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Red, 0.1f))
	{
		//컴포넌트 먼저 구현 검사
		UPrimitiveComponent* Comp = HitResult.GetComponent();
		AActor* Actor = HitResult.GetActor();

		return GetImplementObject(Comp, Actor);
	}
	return nullptr;
}

UObject* UGripComponent::DetectTargetByCollision()
{
	if (TargetObj && bGripByDetectObject)
		return TargetObj;

	if (bDetectByOverlap)
	{
		TArray<UPrimitiveComponent*> DetectedComponents;
		GetOverlappingComponents(DetectedComponents);
		for (UPrimitiveComponent* UPrim : DetectedComponents)
		{
			if (UObject* Obj = GetImplementObject(UPrim, UPrim->GetOwner()))
				return Obj;
		}
	}
	else
	{
		TArray<FHitResult> HitResults;
		const FVector TraceRange = bIsLeft ? this->GetRightVector() * 0.1f : this->GetRightVector() * -0.1f;
		TArray<AActor*> ActorToIgnore{ this->GetOwner() };
		if (UKismetSystemLibrary::BoxTraceMulti(this, GetComponentLocation() - TraceRange, GetComponentLocation() + TraceRange, BoxExtent / 2, GetComponentRotation(),
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			true, ActorToIgnore,
			bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
			HitResults,
			true,
			FLinearColor::Yellow,
			FLinearColor::Red, 0.1f))
		{
			for (FHitResult HitResult : HitResults)
			{
				UPrimitiveComponent* HitComp = HitResult.GetComponent();
				AActor* Actor = HitResult.GetActor();
				if (UObject* Obj = GetImplementObject(HitComp, Actor))
					return Obj;
			}

		}
	}

	return nullptr;
}

UObject* UGripComponent::GetImplementObject(UPrimitiveComponent* Comp, AActor* Actor)
{
	if (Comp->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
	{
		// Interface Execute 함수의 경우 Interface 클래스 생성자가 있어야 함. (.cpp)
		if (!IGripInterface::Execute_bDenyGrip(Comp, this))
			return Comp;
	}

	if (Actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
	{
		if (!IGripInterface::Execute_bDenyGrip(Actor, this))
			return Actor;
	}
	return nullptr;
}

void UGripComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ServerGrippedObject.IsValid() && !LocalGrippedObject.IsValid())
		if (UObject* Obj = GetImplementObject(OtherComp, OtherActor))
		{
			if (TargetObj == Obj)
				return;

			TargetObj = Obj;
			ClearHighlightComps();
			HighlightTarget(Obj);
		}
}

void UGripComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!ServerGrippedObject.IsValid() && !LocalGrippedObject.IsValid())
		if (TargetObj == OtherComp || TargetObj == OtherActor)
		{
			ClearHighlightComps();
			TargetObj = nullptr;
		}
}

void UGripComponent::HighlightTarget(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		if (Actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			if (IGripInterface::Execute_bDenyHighlight(Actor, this))
				return;
			//Primitive Component 찾기
			TArray<UActorComponent*> ActorComps;
			Actor->GetComponents(UPrimitiveComponent::StaticClass(), ActorComps);
			for (UActorComponent* ActorComp : ActorComps)
			{
				//타겟 하이라이팅
				UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ActorComp);
				PrimComp->SetRenderCustomDepth(true);
				HighlightedComps.Add(PrimComp);
			}
			return;
		}
	}
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Object))
	{
		if (PrimComp->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			if (IGripInterface::Execute_bDenyHighlight(PrimComp, this))
				return;
			PrimComp->SetRenderCustomDepth(true);
			HighlightedComps.Add(PrimComp);
		}
	}
}

void UGripComponent::ClearHighlightComps()
{
	if (HighlightedComps.Num())
	{
		for (UPrimitiveComponent* PrimComp : HighlightedComps)
		{
			PrimComp->SetRenderCustomDepth(false);
		}
		HighlightedComps.Empty();
	}
}

/********************************************************/
/****************	*Grip Function*	*********************/
/********************************************************/

void UGripComponent::Grip()
{
	if (DetectionType != EGripDetectionType::Laser)
	{
		if (UObject* GripTarget = DetectTargetByCollision())
		{
			if (AActor* actor = Cast<AActor>(GripTarget))
			{
				TryGrab(GripTarget, ConvertToCompRelativeTransform(actor->GetActorTransform()));
				return;
			}

			if (USceneComponent* comp = Cast<USceneComponent>(GripTarget))
			{
				TryGrab(GripTarget, ConvertToCompRelativeTransform(comp->GetComponentTransform()));
				return;
			}
		}
	}
	if (DetectionType != EGripDetectionType::Collision)
	{
		if (bLaserActivate)
			if (UObject* GripTarget = DetectTargetByLaser())
			{
				if (AActor* actor = Cast<AActor>(GripTarget))
				{
					TryGrab(GripTarget, ConvertToCompRelativeTransform(actor->GetActorTransform()));
					return;
				}
				if (USceneComponent* comp = Cast<USceneComponent>(GripTarget))
				{
					TryGrab(GripTarget, ConvertToCompRelativeTransform(comp->GetComponentTransform()));
					return;
				}
			}
	}
}

bool UGripComponent::TryGrab(UObject* ObjectToGrip, const FTransform& RelativeTransform)
{
	//Grab Rep Mov Type에 맞게 실행 분기
	if (ObjectToGrip->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
	{
		switch (IGripInterface::Execute_GetGripRepMoveType(ObjectToGrip)) {

		case EGripRepMoveType::Server_RepMovement:
			ServerGrabObjectByInterface(ObjectToGrip, RelativeTransform);
			break;
			//서버 RPC 잡기 시동.
		case EGripRepMoveType::Client_CompAttach:
		case EGripRepMoveType::NoRepMove:
			GrabObjectByInterface(ObjectToGrip, RelativeTransform);
			break;
		}
		return true;
	}

	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ObjectToGrip))
	{
		AActor* actor = PrimComp->GetOwner();
		if (actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			switch (IGripInterface::Execute_GetGripRepMoveType(actor)) {

			case EGripRepMoveType::Server_RepMovement:
				ServerGrabObjectByInterface(actor, RelativeTransform);
				break;
				//서버 RPC 잡기 시동.
			case EGripRepMoveType::Client_CompAttach:
			case EGripRepMoveType::NoRepMove:
				GrabObjectByInterface(actor, RelativeTransform);
				break;
			}
			return true;
		}
	}
	return false;
}

bool UGripComponent::GrabObjectByInterface(UObject* ObjectToGrip, const FTransform& RelativeTransform)
{
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ObjectToGrip))
	{
		AActor* Owner = PrimComp->GetOwner();
		if (!Owner)
			return false;
		/*
		 * PrimComp 체크 -> PrimComp 소유 엑터 체크 -> Actor Root Comp 체크 -> Actor 인터페이스 체크
		 */

		 //PrimComp 우선 인터페이스 체크
		if (PrimComp->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_GripComponent(PrimComp, RelativeTransform, IGripInterface::Execute_GetGripType(PrimComp), IGripInterface::Execute_GetGripRepMoveType(PrimComp));
		}
		//PrimComp 소유 액터 인터페이스 체크
		else if (Owner->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_GripActor(Owner, RelativeTransform, IGripInterface::Execute_GetGripType(Owner), IGripInterface::Execute_GetGripRepMoveType(Owner));
		}
		else
		{
			// No interface, no grip
			return false;
		}
	}
	else if (AActor* Actor = Cast<AActor>(ObjectToGrip))
	{
		//Target Obj is PrimComp
		//Valid Check
		UPrimitiveComponent* root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (!root)
			return false;

		//RootComp 인터페이스 체크
		if (root->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_GripComponent(root, RelativeTransform, IGripInterface::Execute_GetGripType(root), IGripInterface::Execute_GetGripRepMoveType(root));
		}
		//액터 인터페이스 체크
		else if (Actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_GripActor(Actor, RelativeTransform, IGripInterface::Execute_GetGripType(Actor), IGripInterface::Execute_GetGripRepMoveType(Actor)
			);
		}
		else
		{
			// No interface, no grip
			return false;
		}
	}
	return false;
}

void UGripComponent::ServerGrabObjectByInterface_Implementation(UObject* ObjectToGrip, const FTransform& RelativeTransform)
{
	GrabObjectByInterface(ObjectToGrip, RelativeTransform);
}

bool UGripComponent::ServerGrabObjectByInterface_Validate(UObject* ObjectToGrip, const FTransform& RelativeTransform)
{
	return true;
}

bool UGripComponent::Event_GripComponent(UPrimitiveComponent* TargetComp, const FTransform& RelativeTransform, EGripType GripType, EGripRepMoveType GripMovementReplicationType)
{
	const bool bIsLocalGrip = (GripMovementReplicationType == EGripRepMoveType::Client_CompAttach || GripMovementReplicationType == EGripRepMoveType::NoRepMove);

	//로컬 그립이 서버에서 이루어질 수 없음.
	if (!IsServer() && !bIsLocalGrip) return false;

	//대상 검증 실패 및 삭제 예정.
	if (!TargetComp) return false;

	//대상이 현재 쥐고 있는 객체와 같다면 이미 쥐고 있고 있음
	if (GetIsObjectHeld(TargetComp))
	{
		return false;
	}

	//커스텀 그립, 이벤트 그립이지만 움직일 수 없다면 집을 수 없음.
	if (TargetComp->Mobility != EComponentMobility::Movable && (GripType != EGripType::CustomGrip && GripType != EGripType::EventGrip))
	{
		return false;
	}

	//인터페이스 구현 체크
	if (TargetComp->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
	{
		bool bIsHeld;
		TArray<UGripComponent*> HoldingComps;

		//Grip Denying 체크
		if (IGripInterface::Execute_bDenyGrip(TargetComp, this))
			return false;

		//Temp : 객체를 다른 사람이 쥐고 있으면 집을 수 없음.
		// TODO : 다른 여러 사람들이 집을 수 있게 구현.. 
		IGripInterface::Execute_IsHeld(TargetComp, HoldingComps, bIsHeld);
		if (bIsHeld)
		{
			return false;
		}
		//TODO: 다중그립 구현
		/*
		bool bAllowMultipleGrips = IGripInterface::Execute_AllowsMultipleGrips(TargetComp);
		if(bIsHeld && !bAllowMultipleGrips)
		{
			return false;
		}
		else if(bIsHeld)
		{
			if(HoldingComps[0].HoldingComp != nullptr)
			{
				//FBPActorGripInformation *GripInfo = HoldingComps[0].HoldingComp->GetGripPtrByID(HoldingComs[0].GripID);

			}
		}
		*/
	}

	TargetComp->AddTickPrerequisiteComponent(this);

	//Grip Info 세팅
	FGripInformation NewCompGrip;
	NewCompGrip.Type = GripType;
	NewCompGrip.TargetType = EGripTargetType::ComponentGrip;
	NewCompGrip.TargetObject = TargetComp;
	if (TargetComp->GetOwner())
	{
		NewCompGrip.bOriginalRepMov = TargetComp->GetOwner()->IsReplicatingMovement();
	}
	NewCompGrip.bOriginalGravity = TargetComp->IsGravityEnabled();
	NewCompGrip.MovementReplicationType = GripMovementReplicationType;
	NewCompGrip.RelativeTransform = RelativeTransform;

	//Networking
	return Event_SetGripInfo(NewCompGrip, bIsLocalGrip);
}

bool UGripComponent::Event_GripActor(AActor* TargetActor, const FTransform& RelativeTransform, EGripType GripType, EGripRepMoveType GripMovementReplicationType)
{
	const bool bIsLocalGrip = (GripMovementReplicationType == EGripRepMoveType::Client_CompAttach || GripMovementReplicationType == EGripRepMoveType::NoRepMove);

	//로컬 그립이 서버에서 이루어질 수 없음.
	if (!IsServer() && !bIsLocalGrip)
	{
		return false;
	}

	//대상 검증 실패 및 삭제 예정.
	if (!TargetActor)
	{
		return false;
	}

	//대상이 현재 쥐고 있는 객체와 같다면 이미 쥐고 있고 있음
	if (GetIsObjectHeld(TargetActor))
	{
		return false;
	}

	USceneComponent* root = Cast<USceneComponent>(TargetActor->GetRootComponent());
	if (!root)
	{
		return false;
	}

	//커스텀 그립, 이벤트 그립이지만 움직일 수 없다면 집을 수 없음.
	if (root->Mobility != EComponentMobility::Movable
		&& (GripType != EGripType::CustomGrip && GripType != EGripType::EventGrip))
	{
		return false;
	}

	TArray<UGripComponent*> HoldingComps;
	bool bIsHeld;

	if (TargetActor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
	{
		if (IGripInterface::Execute_bDenyGrip(TargetActor, this))
			return false;

		IGripInterface::Execute_IsHeld(TargetActor, HoldingComps, bIsHeld);
		if (bIsHeld) {
			return false;
		}
	}

	TargetActor->AddTickPrerequisiteComponent(this);

	//Grip Info 세팅
	FGripInformation NewActorGrip;
	NewActorGrip.Type = GripType;
	NewActorGrip.TargetType = EGripTargetType::ActorGrip;
	NewActorGrip.TargetObject = TargetActor;
	NewActorGrip.bOriginalRepMov = TargetActor->IsReplicatingMovement();
	NewActorGrip.bOriginalGravity = false;
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(root))
		NewActorGrip.bOriginalGravity = PrimComp->IsGravityEnabled();
	NewActorGrip.MovementReplicationType = GripMovementReplicationType;
	NewActorGrip.RelativeTransform = RelativeTransform;

	return Event_SetGripInfo(NewActorGrip, bIsLocalGrip);
}

bool UGripComponent::Event_SetGripInfo(const FGripInformation& GripInfo, bool bIsLocalGrip)
{
	//!!Server Side!!  No Client Side
	if (!bIsLocalGrip)
	{
		ServerGrippedObject = GripInfo;
		Event_SetupGrip(ServerGrippedObject);
	}
	//!!Client Side!! No Server Side
	else
	{
		if (!bHasLocalNetOwner())
			return false;

		LocalGrippedObject = GripInfo;
		Event_SetupGrip(LocalGrippedObject);
		FGripInformation NewLocalGripInfo = LocalGrippedObject;
		if (GetNetMode() == ENetMode::NM_Client && !IsTornOff() && NewLocalGripInfo.MovementReplicationType == EGripRepMoveType::Client_CompAttach)
		{
			ServerNotifyLocalGrip(NewLocalGripInfo);
		}
	}
	return true;
}

bool UGripComponent::Event_SetupGrip(FGripInformation& GripInfomation)
{
	ClearHighlightComps();
	UPrimitiveComponent* root = nullptr;
	AActor* actor = nullptr;

	if (!GripInfomation.TargetObject || !GripInfomation.TargetObject->IsValidLowLevelFast())
		return false;

	if (!(LocalGrippedObject == GripInfomation) && !(ServerGrippedObject == GripInfomation))
		return false;

	//Actor, Component Property 셋업
	switch (GripInfomation.TargetType)
	{
	case EGripTargetType::ActorGrip:
	{
		actor = GripInfomation.GetGrabbedActor();
		if (actor)
		{
			root = Cast<UPrimitiveComponent>(actor->GetRootComponent());

			if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
			{
				if (GripInfomation.Type != EGripType::EventGrip)
				{
					OwningPawn->MoveIgnoreActorAdd(actor);
				}
			}
			IGripInterface::Execute_SetHeld(actor, this, true);
			IGripInterface::Execute_OnGrip(actor, this, GripInfomation);

			if (root)
			{
				if (GripInfomation.Type != EGripType::EventGrip)
				{
					//RepMov시 서버 이외에서는 움직임을 Set해주지 않기 때문에 중력을 꺼야 자연스러운 움직임
					if (GripInfomation.MovementReplicationType == EGripRepMoveType::Server_RepMovement && !IsServer())
						root->SetEnableGravity(false);
				}
			}
		}
		else
			return false;
	}
	break;
	case EGripTargetType::ComponentGrip:
	{
		root = GripInfomation.GetGrabbedComponent();
		if (root)
		{
			actor = root->GetOwner();

			IGripInterface::Execute_SetHeld(root, this, true);
			IGripInterface::Execute_OnGrip(root, this, GripInfomation);

			if (GripInfomation.Type != EGripType::EventGrip)
			{
				//RepMov시 서버 이외에서는 움직임을 Set해주지 않기 때문에 중력을 꺼야 자연스러운 움직임
				if (GripInfomation.MovementReplicationType == EGripRepMoveType::Server_RepMovement && !IsServer())
					root->SetEnableGravity(false);
			}
		}
		else
			return false;
	}
	break;
	}

	//Replication 셋업
	switch (GripInfomation.MovementReplicationType)
	{
	case EGripRepMoveType::Client_CompAttach:
	case EGripRepMoveType::NoRepMove:
	{
		if (GripInfomation.Type != EGripType::EventGrip)
		{
			if (IsServer() && actor && ((GripInfomation.TargetType == EGripTargetType::ActorGrip)
				|| (root && root == actor->GetRootComponent())))
			{
				actor->SetReplicateMovement(false);
			}
			if (root)
			{
				// #TODO: This is a hack until Epic fixes their new physics replication code
				//		  It forces the replication target to null on grip if we aren't repping movement.
				//#if PHYSICS_INTERFACE_PHYSX
				if (UWorld* World = GetWorld())
				{
					if (FPhysScene* PhysScene = World->GetPhysicsScene())
					{
						if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
						{
							FBodyInstance* BI = root->GetBodyInstance();
							if (BI && BI->IsInstanceSimulatingPhysics())
							{
								PhysicsReplication->RemoveReplicatedTarget(root);
							}
						}
					}
				}
				//#endif
			}
		}
	}break;

	case EGripRepMoveType::Server_RepMovement:
	{
		if (GripInfomation.Type != EGripType::EventGrip)
		{
			if (IsServer() &&
				(actor && (GripInfomation.TargetType == EGripTargetType::ActorGrip)
					|| (root && root == actor->GetRootComponent())))
			{
				actor->SetReplicateMovement(true);
			}
		}
	}break;
	default:
	{}break;
	}

	// Movement 셋업
	bool bHasMovementAuthority = HasGripMovementAuthority(GripInfomation);
	switch (GripInfomation.Type)
	{
	case EGripType::PhysicsGrip:
	{
		if (bHasMovementAuthority)
		{
			SetUpPhysicsHandle(GripInfomation);
		}
	}
	break;
	case EGripType::NoPhysicsGrip:
	{
		if (root)
			root->SetSimulatePhysics(false);
		root->SetWorldTransform(GripInfomation.RelativeTransform * this->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);
		root->AttachToComponent(this
			, FAttachmentTransformRules(EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld, true));
	}
	break;
	case EGripType::PhysicsSocketGrip:
	case EGripType::NoPhysicsSocketGrip:
	case EGripType::EventGrip:
	case EGripType::CustomGrip:
		break;
	default:
	{
		if (root)
			root->SetSimulatePhysics(false);
	}
	break;
	}
	if (OnGrippedObject.IsBound())
		OnGrippedObject.Broadcast(GripInfomation);
	return true;
}

void UGripComponent::ServerNotifyLocalGrip_Implementation(const FGripInformation& GripInfomation)
{
	if (!GripInfomation.TargetObject || GripInfomation.MovementReplicationType != EGripRepMoveType::Client_CompAttach)
	{
		return;
	}

	if (!(LocalGrippedObject == GripInfomation))
	{
		UPrimitiveComponent* PrimComp = GripInfomation.GetGrabbedComponent();
		AActor* pActor = GripInfomation.GetGrabbedActor();

		if (!PrimComp && pActor)
		{
			PrimComp = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		}
		else if (!pActor && PrimComp)
		{
			pActor = PrimComp->GetOwner();
		}

		if (!PrimComp || !pActor)
		{
			return;
		}

		TArray<UGripComponent*> HoldingComps;
		bool bIsHeld = false;

		if (GripInfomation.TargetObject->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			IGripInterface::Execute_IsHeld(GripInfomation.TargetObject, HoldingComps, bIsHeld);
			if (bIsHeld)
			{
				/*// If we are held by multiple controllers then lets copy our original values from the first one
				if (HoldingComps.Num() > 0 && HoldingComps[0].HoldingComp != nullptr)
				{
					FGripInformation* gripInfo = HoldingComps[0].HoldingComp->GetGripPtrByID(HoldingComps[0].GripID);

					if (gripInfo != nullptr)
					{
						bHadOriginalSettings = true;
						bOriginalGravity = gripInfo->bOriginalGravity;
						bOriginalReplication = gripInfo->bOriginalReplicatesMovement;
					}
				}*/
				return;
			}
		}

		LocalGrippedObject = GripInfomation;

		if (LocalGrippedObject.IsValid())
		{
			HandleGripReplication(LocalGrippedObject);
		}
	}
	else
	{
		if (LocalGrippedObject == GripInfomation)
		{
			FGripInformation OriginalGrip = LocalGrippedObject;
			LocalGrippedObject.RepCopy(GripInfomation);
			HandleGripReplication(LocalGrippedObject, &OriginalGrip);
		}
	}
}

bool UGripComponent::ServerNotifyLocalGrip_Validate(const FGripInformation& newGrip)
{
	return true;
}

/********************************************************/
/****************	*Drop Function*	*********************/
/********************************************************/
void UGripComponent::TryDrop()
{
	if (LocalGrippedObject.IsValid())
	{
		DropObjectByInterface(LocalGrippedObject.TargetObject);
	}
	else if (ServerGrippedObject.IsValid())
	{
		ServerTryDrop();
	}
}

void UGripComponent::ServerTryDrop_Implementation()
{
	if (ServerGrippedObject.IsValid())
	{
		DropObjectByInterface(ServerGrippedObject.TargetObject);
	}
}

bool UGripComponent::ServerTryDrop_Validate()
{
	return true;
}

bool UGripComponent::DropObjectByInterface(UObject* ObjectToDrop)
{
	FGripInformation* GripInfo = nullptr;

	if (ObjectToDrop == nullptr)
	{
		return false;
	}

	if (ServerGrippedObject == ObjectToDrop)
		GripInfo = &ServerGrippedObject;
	else if (LocalGrippedObject == ObjectToDrop)
		GripInfo = &LocalGrippedObject;

	if (GripInfo == nullptr)
	{
		return false;
	}

	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GripInfo->TargetObject))
	{
		AActor* Owner = PrimComp->GetOwner();

		if (!Owner)
			return false;

		if (PrimComp->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_DropGripInfo(*GripInfo, IGripInterface::Execute_bDropSimulate(PrimComp));
		}
		else if (Owner->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_DropGripInfo(*GripInfo, IGripInterface::Execute_bDropSimulate(Owner));
		}
		else
		{
			return Event_DropGripInfo(*GripInfo, true);
		}
	}
	else if (AActor* Actor = Cast<AActor>(GripInfo->TargetObject))
	{
		UPrimitiveComponent* root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

		if (!root)
			return false;

		if (root->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_DropGripInfo(*GripInfo, IGripInterface::Execute_bDropSimulate(root));
		}
		else if (Actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			return Event_DropGripInfo(*GripInfo, IGripInterface::Execute_bDropSimulate(Actor));
		}
		else
		{
			return Event_DropGripInfo(*GripInfo, true);
		}
	}

	return false;
}

bool UGripComponent::Event_DropGripInfo(const FGripInformation& Grip, bool bSimulate)
{
	bool bIsServer = IsServer();
	bool bWasLocalGrip = false;
	if (!(LocalGrippedObject == Grip))
	{
		if (!bIsServer)
		{
			return false;
		}

		if (!(ServerGrippedObject == Grip))
		{
			return false;
		}
		bWasLocalGrip = false;
	}
	else
		bWasLocalGrip = true;

	UPrimitiveComponent* PrimComp = nullptr;
	AActor* Actor = nullptr;

	if (bWasLocalGrip)
	{
		PrimComp = LocalGrippedObject.GetGrabbedComponent();
		Actor = LocalGrippedObject.GetGrabbedActor();
	}
	else
	{
		PrimComp = ServerGrippedObject.GetGrabbedComponent();
		Actor = ServerGrippedObject.GetGrabbedActor();
	}

	if (!PrimComp && Actor)
		PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());


	if (bWasLocalGrip)
	{
		if (GetNetMode() == ENetMode::NM_Client)
		{
			if (!IsTornOff())
			{
				//FTransform_NetQuantize
				FTransform TransformAtDrop = FTransform::Identity;

				switch (LocalGrippedObject.TargetType)
				{
				case EGripTargetType::ActorGrip:
				{
					if (AActor* GrabbedActor = LocalGrippedObject.GetGrabbedActor())
					{
						TransformAtDrop = GrabbedActor->GetActorTransform();
					}
				}
				break;
				case EGripTargetType::ComponentGrip:
				{
					if (UPrimitiveComponent* GrabbedPrim = LocalGrippedObject.GetGrabbedComponent())
					{
						TransformAtDrop = GrabbedPrim->GetComponentTransform();
					}
				}
				break;
				default:
					break;
				}
				ServerNotifyLocalGripRemoved(TransformAtDrop);
			}
			Drop_Implementation(LocalGrippedObject, bSimulate);
		}
		else
		{
			Multicast_NotifyDrop(LocalGrippedObject, bSimulate);
		}
	}
	else
		Multicast_NotifyDrop(ServerGrippedObject, bSimulate);

	return true;
}

void UGripComponent::Drop_Implementation(const FGripInformation& NewDrop, bool bSimulate)
{
	TArray<UGripComponent*> HoldingComps;
	bool bIsHeld = false;

	if (NewDrop.TargetObject && NewDrop.TargetObject->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		IGripInterface::Execute_IsHeld(NewDrop.TargetObject, HoldingComps, bIsHeld);

	DestroyPhysicsHandle(NewDrop);

	bool bHadGripAuthority = HasGripAuthority(NewDrop);
	UPrimitiveComponent* Root = nullptr;
	AActor* Actor = nullptr;

	switch (NewDrop.TargetType)
	{
	case EGripTargetType::ActorGrip:
	{
		Actor = NewDrop.GetGrabbedActor();
		if (Actor)
		{
			Root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
			Actor->RemoveTickPrerequisiteComponent(this);
			if (NewDrop.Type != EGripType::EventGrip)
			{
				if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
				{
					OwningPawn->MoveIgnoreActorRemove(Actor);
				}
			}

			if (Root)
			{
				if (NewDrop.Type == EGripType::NoPhysicsGrip)
					Root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				if (NewDrop.Type != EGripType::EventGrip)
				{
					if (IsServer() || bHadGripAuthority || !NewDrop.bOriginalRepMov || !Actor->GetIsReplicated())
					{
						if (Root->IsSimulatingPhysics() != bSimulate)
							Root->SetSimulatePhysics(bSimulate);

						if (bSimulate)
							Root->WakeAllRigidBodies();
					}
					Root->UpdateComponentToWorld();

					if (NewDrop.MovementReplicationType == EGripRepMoveType::Server_RepMovement && !IsServer())
						Root->SetEnableGravity(NewDrop.bOriginalGravity);
				}
			}
		}
		if (IsServer())
		{
			Actor->SetReplicateMovement(NewDrop.bOriginalRepMov);
		}

		if (Actor->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
		{
			IGripInterface::Execute_SetHeld(Actor, this, false);
			IGripInterface::Execute_OnGripRelease(Actor, this, NewDrop);
		}
	}

	break;
	case EGripTargetType::ComponentGrip:
	{
		Root = NewDrop.GetGrabbedComponent();
		if (Root)
		{
			Actor = Root->GetOwner();
			Root->RemoveTickPrerequisiteComponent(this);

			if (NewDrop.Type == EGripType::NoPhysicsGrip)// && (IsServer() || HasGripAuthority(NewDrop)))
				Root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

			if (NewDrop.Type != EGripType::EventGrip)
			{
				if (IsServer() || bHadGripAuthority || !NewDrop.bOriginalRepMov || (Actor && (Actor->GetRootComponent() != Root || !Actor->GetIsReplicated())))
				{
					if (Root->IsSimulatingPhysics() != bSimulate)
						Root->SetSimulatePhysics(bSimulate);
					if (bSimulate)
						Root->WakeAllRigidBodies();

				}
			}
			Root->UpdateComponentToWorld();
			if ((NewDrop.MovementReplicationType == EGripRepMoveType::Server_RepMovement && !IsServer()))
				Root->SetEnableGravity(NewDrop.bOriginalGravity);


			if (Actor)
			{
				if (IsServer() && Root == Actor->GetRootComponent())
				{
					Actor->SetReplicateMovement(NewDrop.bOriginalRepMov);
				}
			}

			if (Root->GetClass()->ImplementsInterface(UGripInterface::StaticClass()))
			{
				IGripInterface::Execute_SetHeld(Root, this, false);
				IGripInterface::Execute_OnGripRelease(Root, this, NewDrop);
			}
		}
	}
	break;
	}

	switch (NewDrop.MovementReplicationType)
	{
	case EGripRepMoveType::Client_CompAttach:
	case EGripRepMoveType::NoRepMove:
	{
		if (NewDrop.Type != EGripType::EventGrip)
		{
			if (Root)
			{
				// #TODO: This is a hack until Epic fixes their new physics replication code
				//		  It forces the replication target to null on grip if we aren't repping movement.
				//#if PHYSICS_INTERFACE_PHYSX
				if (UWorld* World = GetWorld())
				{
					if (FPhysScene* PhysScene = World->GetPhysicsScene())
					{
						if (IPhysicsReplication* PhysicsReplication = PhysScene->GetPhysicsReplication())
						{
							FBodyInstance* BI = Root->GetBodyInstance();
							if (BI && BI->IsInstanceSimulatingPhysics())
							{
								PhysicsReplication->RemoveReplicatedTarget(Root);
							}
						}
					}
				}
				//#endif
			}
		}

	}break;
	default:
		break;
	}
	
	FGripInformation DropBroadcastData = NewDrop;
	LocalGrippedObject.Reset();
	ServerGrippedObject.Reset();
	bWasInitiallyRepped = false;

	if (OnDroppedObject.IsBound())
		OnDroppedObject.Broadcast(DropBroadcastData);//, false);
}

void UGripComponent::Multicast_NotifyDrop_Implementation(const FGripInformation& NewDrop, bool bSimulate)
{
	Drop_Implementation(NewDrop, bSimulate);
}

void UGripComponent::ServerNotifyLocalGripRemoved_Implementation(const FTransform& TransformAtDrop)
{
	if (!LocalGrippedObject.IsValid())
		return;
	switch (LocalGrippedObject.TargetType)
	{
	case EGripTargetType::ActorGrip:
		LocalGrippedObject.GetGrabbedActor()->SetActorTransform(TransformAtDrop, false, nullptr, ETeleportType::TeleportPhysics); break;
	case EGripTargetType::ComponentGrip:
		LocalGrippedObject.GetGrabbedComponent()->SetWorldTransform(TransformAtDrop, false, nullptr, ETeleportType::TeleportPhysics); break;
	default:break;
	}
	Event_DropGripInfo(LocalGrippedObject, IGripInterface::Execute_bDropSimulate(LocalGrippedObject.TargetObject));
}

bool UGripComponent::ServerNotifyLocalGripRemoved_Validate(const FTransform& TransformAtDrop)
{
	return true;
}

/********************************************************/
/****************	*Physics Handle* 	****************/
/********************************************************/

bool UGripComponent::SetUpPhysicsHandle(const FGripInformation& GripInfomation)
{
	UPrimitiveComponent* root = GripInfomation.GetGrabbedComponent();
	AActor* actor = GripInfomation.GetGrabbedActor();

	if (!root && actor)
		root = Cast<UPrimitiveComponent>(actor->GetRootComponent());
	if (!root)
		return false;

	// HandleInfo == PhysicsGrp.HandleObject
	if (PhysicsGrip.HandledObject == nullptr)
	{
		PhysicsGrip.HandledObject = GripInfomation.TargetObject;
	}

	root->SetSimulatePhysics(true);

	FBodyInstance* RefBodyInstance = root->GetBodyInstance();
	if (!RefBodyInstance || !RefBodyInstance->IsValidBodyInstance() ||
		!FPhysicsInterface::IsValid(RefBodyInstance->ActorHandle) || !RefBodyInstance->BodySetup.IsValid())
	{
		return false;
	}
	check(RefBodyInstance->BodySetup->GetCollisionTraceFlag() != CTF_UseComplexAsSimple);

	if (!FPhysicsInterface::IsValid(PhysicsGrip.KinActorData) && !RefBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
	{
		RefBodyInstance->UpdateMassProperties();
	}

	FPhysicsCommand::ExecuteWrite(RefBodyInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
	{
		FTransform KinPose;
		FTransform trans = FPhysicsInterface::GetGlobalPose_AssumesLocked(Actor);
		KinPose = trans;
		bool bRecreatingConstraint = false;

		if (!FPhysicsInterface::IsValid(PhysicsGrip.KinActorData))
		{
			FActorCreationParams ActorParams;
			ActorParams.InitialTM = KinPose;
			ActorParams.DebugName = nullptr;
			ActorParams.bEnableGravity = false;
			ActorParams.bQueryOnly = false;
			ActorParams.bStatic = false;
			ActorParams.Scene = FPhysicsInterface::GetCurrentScene(Actor);
			FPhysicsInterface::CreateActor(ActorParams, PhysicsGrip.KinActorData);

			if (FPhysicsInterface::IsValid(PhysicsGrip.KinActorData))
			{
				FPhysicsInterface::SetMass_AssumesLocked(PhysicsGrip.KinActorData, 1.0f);
				FPhysicsInterface::SetMassSpaceInertiaTensor_AssumesLocked(PhysicsGrip.KinActorData, FVector(1.f));
				FPhysicsInterface::SetIsKinematic_AssumesLocked(PhysicsGrip.KinActorData, true);
				FPhysicsInterface::SetMaxDepenetrationVelocity_AssumesLocked(PhysicsGrip.KinActorData, MAX_FLT);
			}
			
			using namespace Chaos;
			PhysicsGrip.KinActorData->GetGameThreadAPI().SetGeometry(TUniquePtr<FImplicitObject>(new TSphere<FReal, 3>(TVector<FReal, 3>(0.f), 1000.f)));
			PhysicsGrip.KinActorData->GetGameThreadAPI().SetObjectState(EObjectStateType::Kinematic);
			FPhysicsInterface::AddActorToSolver(PhysicsGrip.KinActorData, ActorParams.Scene->GetSolver());
		}

		if (!PhysicsGrip.HandleData.IsValid())
		{
			PhysicsGrip.HandleData = FPhysicsInterface::CreateConstraint(PhysicsGrip.KinActorData, Actor, FTransform::Identity, KinPose.GetRelativeTransform(FPhysicsInterface::GetGlobalPose_AssumesLocked(Actor)));
		}
		else
		{
			bRecreatingConstraint = true;
			FPhysicsInterface::ReleaseConstraint(PhysicsGrip.HandleData);
			PhysicsGrip.HandleData = FPhysicsInterface::CreateConstraint(PhysicsGrip.KinActorData, Actor, FTransform::Identity, KinPose.GetRelativeTransform(FPhysicsInterface::GetGlobalPose_AssumesLocked(Actor)));
		}

		if (PhysicsGrip.HandleData.IsValid())
		{
			FPhysicsInterface::SetBreakForces_AssumesLocked(PhysicsGrip.HandleData, MAX_FLT, MAX_FLT);
			FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::X, ELinearConstraintMotion::LCM_Free);
			FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::Y, ELinearConstraintMotion::LCM_Free);
			FPhysicsInterface::SetLinearMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::Z, ELinearConstraintMotion::LCM_Free);
			FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::Twist, EAngularConstraintMotion::ACM_Free);
			FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::Swing1, EAngularConstraintMotion::ACM_Free);
			FPhysicsInterface::SetAngularMotionLimitType_AssumesLocked(PhysicsGrip.HandleData, PhysicsInterfaceTypes::ELimitAxis::Swing2, EAngularConstraintMotion::ACM_Free);
			FPhysicsInterface::SetDrivePosition(PhysicsGrip.HandleData, FVector::ZeroVector);

			bool bUseForceDrive = false;
			float Stiffness = DEFAULT_STIFFNESS;
			float Damping = DEFAULT_DAMPING;
			float MaxForce;
			float AngularStiffness;
			float AngularDamping;
			float AngularMaxForce;
		
			AngularStiffness = Stiffness * ANGULAR_STIFFNESS_MULTIPLIER; // Default multiplier
			AngularDamping = Damping * ANGULAR_DAMPING_MULTIPLIER; // Default multiplier
			AngularMaxForce = (float)FMath::Clamp<double>((double)AngularStiffness * 0.f, 0, (double)MAX_FLT);
			MaxForce = (float)FMath::Clamp<double>((double)Stiffness * 0.f, 0, (double)MAX_FLT);

			// Different settings for manip grip
			if (!bRecreatingConstraint)
			{
				FConstraintDrive NewLinDrive;
				NewLinDrive.bEnablePositionDrive = true;
				NewLinDrive.bEnableVelocityDrive = true;
				NewLinDrive.Damping = Damping;
				NewLinDrive.Stiffness = Stiffness;
				NewLinDrive.MaxForce = MaxForce;

				FConstraintDrive NewAngDrive;
				NewAngDrive.bEnablePositionDrive = true;
				NewAngDrive.bEnableVelocityDrive = true;
				NewAngDrive.Damping = AngularDamping;
				NewAngDrive.Stiffness = AngularStiffness;
				NewAngDrive.MaxForce = AngularMaxForce;

				PhysicsGrip.LinearConstraint.bEnablePositionDrive = true;
				PhysicsGrip.LinearConstraint.XDrive = NewLinDrive;
				PhysicsGrip.LinearConstraint.YDrive = NewLinDrive;
				PhysicsGrip.LinearConstraint.ZDrive = NewLinDrive;

				PhysicsGrip.AngularConstraint.AngularDriveMode = EAngularDriveMode::SLERP;
				PhysicsGrip.AngularConstraint.SlerpDrive = NewAngDrive;
			}

			FPhysicsInterface::UpdateLinearDrive_AssumesLocked(PhysicsGrip.HandleData, PhysicsGrip.LinearConstraint);
			FPhysicsInterface::UpdateAngularDrive_AssumesLocked(PhysicsGrip.HandleData, PhysicsGrip.AngularConstraint);
		}
	});
	
	if (!RefBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
	{
		RefBodyInstance->OnRecalculatedMassProperties().AddUObject(this, &UGripComponent::OnGripMassUpdated);
	}
	return true;
}

bool UGripComponent::DestroyPhysicsHandle(FGripPhysicsHandleInformation& HandleInfo)
{
	if (!HandleInfo.HandledObject)
		return false;

	FPhysicsInterface::ReleaseConstraint(HandleInfo.HandleData);
	FPhysicsInterface::ReleaseActor(HandleInfo.KinActorData, FPhysicsInterface::GetCurrentScene(HandleInfo.KinActorData));
	PhysicsGrip.Reset();

	return true;
}

bool UGripComponent::DestroyPhysicsHandle(const FGripInformation& Grip)
{
	FGripPhysicsHandleInformation& HandleInfo = PhysicsGrip;
	if (!HandleInfo.HandledObject)
		return true;

	UPrimitiveComponent* Root = Grip.GetGrabbedComponent();
	AActor* Actor = Grip.GetGrabbedActor();
	if (!Root && Actor)
		Root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

	if (Root)
	{
		if (FBodyInstance* RefBodyInstance = Root->GetBodyInstance())
		{
			if (RefBodyInstance->OnRecalculatedMassProperties().IsBoundToObject(this))
			{
				RefBodyInstance->OnRecalculatedMassProperties().RemoveAll(this);
			}
		}
	}

	DestroyPhysicsHandle(HandleInfo);
	PhysicsGrip.Reset();
	
	return true;
}

void UGripComponent::OnGripMassUpdated(FBodyInstance* GripBodyInstance)
{
	TArray<FGripInformation> GripArray;
	this->GetAllGrips(GripArray);
	FGripInformation NewGrip;

	for (int i = 0; i < GripArray.Num(); i++)
	{
		NewGrip = GripArray[i];

		UPrimitiveComponent* root = NewGrip.GetGrabbedComponent();
		AActor* pActor = NewGrip.GetGrabbedActor();

		if (!root && pActor)
		{
			if (!IsValid(pActor)) continue;
			root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
		}

		if (!root || root != GripBodyInstance->OwnerComponent) continue;
		if (IsValid(root))
		{
			UpdatePhysicsHandle(NewGrip, false);
		}
		break;
	}
}

void UGripComponent::GetAllGrips(TArray<FGripInformation>& GripArray)
{
	GripArray.Add(ServerGrippedObject);
	GripArray.Add(LocalGrippedObject);
}

bool UGripComponent::UpdatePhysicsHandle(const FGripInformation& GripInfo, bool bFullyRecreate)
{
	if (!PhysicsGrip.HandledObject)
		return false;

	if (bFullyRecreate)
	{
		return SetUpPhysicsHandle(GripInfo);
	}

	// Not fully recreating
	UPrimitiveComponent* root = GripInfo.GetGrabbedComponent();
	AActor* pActor = GripInfo.GetGrabbedActor();

	if (!root && pActor) root = Cast<UPrimitiveComponent>(pActor->GetRootComponent());
	if (!root) return false;
	
	FBodyInstance* rBodyInstance = root->GetBodyInstance();
	if (!rBodyInstance || !rBodyInstance->IsValidBodyInstance() || !FPhysicsInterface::IsValid(rBodyInstance->ActorHandle))
	{
		return false;
	}
	check(rBodyInstance->BodySetup->GetCollisionTraceFlag() != CTF_UseComplexAsSimple);

	FPhysicsCommand::ExecuteWrite(rBodyInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			if (PhysicsGrip.HandledObject)
			{
				if (PhysicsGrip.KinActorData && FPhysicsInterface::IsValid(PhysicsGrip.KinActorData))
				{
					Chaos::FConstraintBase* ConstraintHandle = PhysicsGrip.HandleData.Constraint;
					if (ConstraintHandle)
					{
						((Chaos::FJointConstraint*)ConstraintHandle)->SetParticleProxies({ PhysicsGrip.KinActorData, Actor});
					}
				}

				//if (physx::PxRigidDynamic* PActor = FPhysicsInterface::GetPxRigidDynamic_AssumesLocked(Actor))
				//{
				//	if (PhysicsGrip.HandleData.IsValid() && PhysicsGrip.HandleData.Constraint)
				//		PhysicsGrip.HandleData.Constraint->setActors(PActor, FPhysicsInterface::GetPxRigidDynamic_AssumesLocked(PhysicsGrip.KinActorData));
				//}
			}
		});
	return true;
}

void UGripComponent::UpdatePhysicsHandleTransform(const FGripInformation& GrabbedActor, const FTransform& NewTransform)
{
	if (!GrabbedActor.TargetObject)
		return;

	if (!PhysicsGrip.HandledObject || !FPhysicsInterface::IsValid(PhysicsGrip.KinActorData))
	{
		SetUpPhysicsHandle(GrabbedActor);
		return;
	}

	if (!PhysicsGrip.LastPhysicsTransform.EqualsNoScale(NewTransform))
	{
		PhysicsGrip.LastPhysicsTransform = NewTransform;
		PhysicsGrip.LastPhysicsTransform.SetScale3D(FVector(1.0f));
		FPhysicsActorHandle ActorHandle = PhysicsGrip.KinActorData;
		FTransform NewTrans = (PhysicsGrip.RootBoneRotation * PhysicsGrip.LastPhysicsTransform);

		FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			FPhysicsInterface::SetKinematicTarget_AssumesLocked(Actor, NewTrans);
		});
	}
}


