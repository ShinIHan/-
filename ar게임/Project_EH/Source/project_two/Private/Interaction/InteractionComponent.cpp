#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractionInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	InterfaceStaticClass = UInteractionInterface::StaticClass();
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		SetComponentTickEnabled(true);
	}
}

void UInteractionComponent::HandleDetection()
{
	if (UObject* Obj = DetectTarget())
	{
		if (TargetObject == Obj)
			return;

		TargetObject = Obj;
		OnDetect.Broadcast(Obj);
		ClearHighlightComps();
		HighlightTarget(Obj);
	}
	else
	{
		if (TargetObject)
		{
			if (IsExecuting)
				EndExecuteInteraction();
			OnEndDetect.Broadcast(TargetObject);
			ClearHighlightComps();
			TargetObject = nullptr;
		}
	}
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (TargetObject)
	{
		if (IsExecuting)
		{
			TickOnExec.Broadcast(TargetObject, DeltaTime);
			IInteractionInterface::Execute_TickOnInteract(TargetObject, this, DeltaTime);
		}
		else
		{
			TickOnDetect.Broadcast(TargetObject, DeltaTime);
			IInteractionInterface::Execute_TickOnDetected(TargetObject, this, DeltaTime);
		}
	}
	HandleDetection();
}

bool UInteractionComponent::ExecuteInteraction()
{
	//Target Implement 검사
	HandleDetection();

	if (!TargetObject || !TargetObject->GetClass()->ImplementsInterface(InterfaceStaticClass))
		return false;
	if (IInteractionInterface::Execute_DenyInteraction(TargetObject, this))
		return false;
	OnExec.Broadcast(TargetObject);
	IInteractionInterface::Execute_OnInteract(TargetObject, this);

	//ExecuteTick On
	IsExecuting = true;
	return true;
}

bool UInteractionComponent::EndExecuteInteraction()
{
	if (!TargetObject || !TargetObject->GetClass()->ImplementsInterface(InterfaceStaticClass))
		return false;

	OnEndExec.Broadcast(TargetObject);
	IInteractionInterface::Execute_OnEndInteract(TargetObject, this);

	//ExecuteTick Off
	IsExecuting = false;
	return true;
}

UObject* UInteractionComponent::DetectTarget()
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
		if (Comp->GetClass()->ImplementsInterface(InterfaceStaticClass))
		{
			if (!GetDenyInteraction(Comp))
				return Comp;
		}
		//엑터 구현시 
		AActor* Actor = HitResult.GetActor();
		if (Actor->GetClass()->ImplementsInterface(InterfaceStaticClass))
		{
			if (!GetDenyInteraction(Actor))
				return Actor;
		}
	}
	return nullptr;
}

bool UInteractionComponent::GetDenyHighlight(UObject* Object)
{
	return IInteractionInterface::Execute_DenyHighlighting(Object, this);
}

bool UInteractionComponent::GetDenyInteraction(UObject* Object)
{
	return IInteractionInterface::Execute_DenyInteraction(Object, this);
}

//Highlight Funcs
void UInteractionComponent::HighlightTarget(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		if (Actor->GetClass()->ImplementsInterface(InterfaceStaticClass))
		{
			if (GetDenyHighlight(Actor))
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
		if (PrimComp->GetClass()->ImplementsInterface(InterfaceStaticClass))
		{
			if (GetDenyHighlight(PrimComp))
				return;
			PrimComp->SetRenderCustomDepth(true);
			HighlightedComps.Add(PrimComp);
		}
	}
}

void UInteractionComponent::ClearHighlightComps()
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