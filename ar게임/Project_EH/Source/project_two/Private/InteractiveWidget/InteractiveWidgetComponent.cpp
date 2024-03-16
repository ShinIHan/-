// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveWidget/InteractiveWidgetComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void UInteractiveWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractiveWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ShouldDrawWidget() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	//Reference Checking 폰이 변경될 수 있기 때문에 체크..
	if (!PlayerCameraManager)
	{
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	}
}
