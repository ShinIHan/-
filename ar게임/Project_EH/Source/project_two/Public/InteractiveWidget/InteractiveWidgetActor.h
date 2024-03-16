// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveWidgetComponent.h"
#include "GameFramework/Actor.h"
#include "InteractiveWidgetActor.generated.h"

/*
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.05.02.
 * ���۷��� : VRExpansionPlugin, Project_One
 *
*/

UENUM(BlueprintType)
enum class EAttachmentTypeForWidget : uint8
{
	NONE,
	AttachToCamera,
	AttachToCharacter
};


UCLASS(Blueprintable, ClassGroup = "InteractiveWidget")
class PROJECT_TWO_API AInteractiveWidgetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractiveWidgetActor();

	virtual void Tick(float DeltaTime) override;
	void Attach();

protected:
	virtual void BeginPlay() override;

// Var
public:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//UStaticMeshComponent* FrameStaticMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* DefaultSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInteractiveWidgetComponent* InteractiveWidgetComp;

	UPROPERTY()
	APawn* FollowingPawn = nullptr;

	UPROPERTY()
	APlayerCameraManager* PlayerCameraManager = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = InteractiveWidget)
	EAttachmentTypeForWidget AttachmentType;
};
