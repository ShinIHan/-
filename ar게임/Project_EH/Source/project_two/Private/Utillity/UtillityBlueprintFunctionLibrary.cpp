// Fill out your copyright notice in the Description page of Project Settings.


#include "Utillity/UtillityBlueprintFunctionLibrary.h"

void UUtillityBlueprintFunctionLibrary::DrawDebugAxes(const FTransform& Transform, float ArrowLength, bool IsDrawSphere, float SphereRadius, const FColor& SphereColor)
{
	const FVector Location = Transform.GetLocation();
	const FQuat Rotation = Transform.GetRotation();

	// Forward X
	DrawDebugDirectionalArrow(GWorld, Location, Location + (Rotation.GetForwardVector() * ArrowLength),	0, FColor::Red,		false, -1, 0, 0.75);
	// Right Y
	DrawDebugDirectionalArrow(GWorld, Location,	Location + (Rotation.GetRightVector() * ArrowLength),	0, FColor::Green,	false, -1, 0, 0.75);
	// Up Z
	DrawDebugDirectionalArrow(GWorld, Location,	Location + (Rotation.GetUpVector() * ArrowLength),		0, FColor::Blue,	false, -1, 0, 0.75);
	
	if (IsDrawSphere)
		DrawDebugSphere(GWorld, Location, SphereRadius, 12, SphereColor);
}
