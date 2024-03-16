// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveWidget/InteractiveWidgetActor.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AInteractiveWidgetActor::AInteractiveWidgetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	//FrameStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WidgetFrameStaticMeshComponent"));
	//RootComponent = FrameStaticMeshComp;

	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(("DefaultSceneComponent"));
	RootComponent = DefaultSceneComponent;

	InteractiveWidgetComp = CreateDefaultSubobject<UInteractiveWidgetComponent>(TEXT("InteractiveWidgetComponent"));
	InteractiveWidgetComp->SetupAttachment(DefaultSceneComponent);
	InteractiveWidgetComp->SetPivot(FVector2D(0.5, 0.5));
	//InteractiveWidgetComp->SetRelativeLocation(FVector(0, 0, 200));
}

// Called when the game starts or when spawned
void AInteractiveWidgetActor::BeginPlay()
{
	Super::BeginPlay();
	
	FollowingPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	if (EAttachmentTypeForWidget::NONE < AttachmentType)
	{
		Attach();
	}
}

// Called every frame
void AInteractiveWidgetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FollowingPawn || !PlayerCameraManager)
	{
		FollowingPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

		if (EAttachmentTypeForWidget::NONE < AttachmentType )
		{
			Attach();
		}
	}
}

void AInteractiveWidgetActor::Attach()
{
	switch (AttachmentType)
	{
	case EAttachmentTypeForWidget::AttachToCamera:
		if (PlayerCameraManager)
			AttachToComponent(PlayerCameraManager->GetTransformComponent(), FAttachmentTransformRules::KeepWorldTransform);
		break;
	case EAttachmentTypeForWidget::AttachToCharacter:
		if (FollowingPawn)
			AttachToComponent(FollowingPawn->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		break;
	}
}