#include "InverseKinematics/VRInverseKinematicsComponent.h"

#include "Utillity/UtillityBlueprintFunctionLibrary.h"
//#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UVRInverseKinematicsComponent::UVRInverseKinematicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetTickGroup(TG_DuringPhysics);
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UVRInverseKinematicsComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	HeadHandAngleLimitDot = FMath::Cos(FMath::DegreesToRadians(HeadHandAngleLimit));
}

void UVRInverseKinematicsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UVRInverseKinematicsComponent, UpperBodyIKData, COND_SkipOwner);
}

// Called every frame
void UVRInverseKinematicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CachedDeltaTime = DeltaTime;

	//IK Solver
	ConvertTransforms();
	SetShoulder();
	SetUpperArms();
	SolveArms();

	//IK Updater
	UpdateIKData();
	ServerUpdateIKData(UpperBodyIKData);

	if (bDebugDraw)
		DrawDebug();
}

void UVRInverseKinematicsComponent::InitialzieBoneLenghtToSkeletalMesh(USkeletalMeshComponent* SkeletalMeshComp)
{
	if (bHasLocalNetOwner())
	{
		SetComponentTickEnabled(true);

		// Skeletal Mesh에 IK_Top Socket 이 존재해야 함
		Height = SkeletalMeshComp->GetSocketLocation("IK_Top").Size();

		// Neck Offset Calc
		const FVector SK_HeadLocation = SkeletalMeshComp->GetSocketLocation("Bip001-Head");
		const FVector SK_NeckLocation = SkeletalMeshComp->GetSocketLocation("Bip001-Neck");
		NeckOffset = (SK_HeadLocation - SK_NeckLocation) * -1;

		// LeftUpperArm 과 RightUpperArm Distance Calc
		const FVector SK_LeftUpperArmLocation = SkeletalMeshComp->GetSocketLocation("Bip001-L-UpperArm");
		const FVector SK_RightUpperArmLocation = SkeletalMeshComp->GetSocketLocation("Bip001-R-UpperArm");
		UpperArmsDistance = FVector::Dist(SK_LeftUpperArmLocation, SK_RightUpperArmLocation);

		// UpperArm 의 길이, LowerArm의 길이, 전체 팔 길이 Calc
		const FVector SK_LeftLowerArmLocation = SkeletalMeshComp->GetSocketLocation("Bip001-L-Forearm");
		const FVector SK_LeftHandLocation = SkeletalMeshComp->GetSocketLocation("Bip001-L-Hand");
		UpperArmLength = FVector::Dist(SK_LeftUpperArmLocation, SK_LeftLowerArmLocation);
		LowerArmLength = FVector::Dist(SK_LeftLowerArmLocation, SK_LeftHandLocation);
		ArmLength = UpperArmLength + LowerArmLength;

		// Clavicle Offset Calc
		const FVector SK_LeftClavicleLocation = SkeletalMeshComp->GetSocketLocation("Bip001-L-Clavicle");
		ClavicleOffset = SK_LeftClavicleLocation.Z - SK_LeftUpperArmLocation.Z;
	}
}

void UVRInverseKinematicsComponent::SetEffectorTransforms(const FTransform& Origin, const FTransform& Head, const FTransform& Left, const FTransform& Right)
{
	// Blueprint 에서 받아옴
	Origin_World = Origin;
	HeadEffector_World = Head;
	LeftEffector_World = Left;
	RightEffector_World = Right;
}

// ServerUpdateIKData
void UVRInverseKinematicsComponent::ServerUpdateIKData_Implementation(FUpperBodyIKData ClientIKData)
{
	UpperBodyIKData = ClientIKData;
}

bool UVRInverseKinematicsComponent::ServerUpdateIKData_Validate(FUpperBodyIKData ClientIKData)
{
	return true;
}

/*

Calc IK 

*/

void UVRInverseKinematicsComponent::ConvertTransforms()
{
	Origin_Local = Origin_World.Inverse();
	Shoulder_World = Shoulder_Local * Origin_World;
	Shoulder_Shoulder = Shoulder_World.Inverse();

	LeftHand_World = AddLocalOffset(LeftEffector_World, HandOffset);
	LeftHand_World.SetRotation(LeftHand_World.Rotator().Add(HandRotationOffset.Pitch, HandRotationOffset.Yaw, HandRotationOffset.Roll).Quaternion());
	
	RightHand_World = AddLocalOffset(RightEffector_World, HandOffset * FVector(1.f, -1.f, 1.f));
	RightHand_World.SetRotation(RightHand_World.Rotator().Add(HandRotationOffset.Pitch, HandRotationOffset.Yaw, HandRotationOffset.Roll).Quaternion());

	Head_Local = HeadEffector_World * Origin_Local;
	LeftHand_Local = LeftHand_World * Origin_Local;
	RightHand_Local = RightHand_World * Origin_Local;
}

FTransform UVRInverseKinematicsComponent::AddLocalOffset(const FTransform& Transform, const FVector Offset)
{
	FTransform Inversed = Transform.Inverse();
	Inversed.AddToTranslation(Offset);
	return Inversed.Inverse();
}

void UVRInverseKinematicsComponent::SetShoulder()
{
	FRotator RotationFromHead = FRotator(0, Head_Local.Rotator().Yaw, 0);
	FRotator RotationFromHands = GetShoulderRotationFromHands();

	FRotator Rotation = FRotator(0.f, UKismetMathLibrary::RLerp(RotationFromHead, RotationFromHands, ShoulderHeadHandAlpha, true).Yaw, 0.f);
	FVector Translation = GetShoulderLocation();
	Shoulder_Local = FTransform(Rotation, Translation, FVector::OneVector);

	Shoulder_World = Shoulder_Local * Origin_World;
	Head_Shoulder = HeadEffector_World * Shoulder_Shoulder;
	LeftHand_Shoulder = LeftHand_World * Shoulder_Shoulder;
	RightHand_Shoulder = RightHand_World * Shoulder_Shoulder;
}

FRotator UVRInverseKinematicsComponent::GetShoulderRotationFromHands()
{
	FVector Offset = FVector(0.f, 0.f, 15.f);
	FVector TopHeadTranslation = HeadEffector_World.GetTranslation() + HeadEffector_World.Rotator().RotateVector(Offset);

	//LEFT HAND
	FVector Hand = (LeftHand_World * HeadEffector_World.Inverse()).GetTranslation();
	FVector HandHeadDelta = LeftHand_World.GetTranslation() - TopHeadTranslation;
	LeftHeadHandAngle = GetHeadHandAngle(LeftHeadHandAngle, Hand, HandHeadDelta);

	//RIGHT Hand
	Hand = (RightHand_World * HeadEffector_World.Inverse()).GetTranslation();
	HandHeadDelta = RightHand_World.GetTranslation() - TopHeadTranslation;
	RightHeadHandAngle = GetHeadHandAngle(RightHeadHandAngle, Hand, HandHeadDelta);

	FRotator YawRotator = FRotator(0.f, ((LeftHeadHandAngle + RightHeadHandAngle) / 2.f), 0.f);
	FTransform TempTransform = FTransform(YawRotator, FVector::ZeroVector, FVector::OneVector);

	//Return In WorldSpace

	TempTransform = TempTransform * HeadEffector_World;
	TempTransform = TempTransform * Origin_Local;

	return TempTransform.Rotator();
}

float UVRInverseKinematicsComponent::GetHeadHandAngle(float LastAngle, const FVector& Hand, const FVector& HandHeadDelta) const
{
	float HeadHandAlpha = UKismetMathLibrary::MapRangeClamped(FVector(HandHeadDelta.X, HandHeadDelta.Y, 0.f).Size(), 20.f, 50.f, 0.f, 1.f);
	FVector Hand2D = FVector(Hand.X, Hand.Y, 0.f);
	FVector Hand2DNormalized = Hand2D.GetSafeNormal();
	float Angle = FMath::RadiansToDegrees(FMath::Atan2(Hand2DNormalized.Y, Hand2DNormalized.X));

	bool Selector1 = FVector::DotProduct(Hand2DNormalized, FVector::ForwardVector) > HeadHandAngleLimitDot;
	bool Selector2 = (FMath::Sign(LastAngle) == FMath::Sign(Angle)) ||
		(Angle < OkSpanAngle&& Angle > -1.0f * OkSpanAngle);
	bool Selector = Selector1 && Selector2;

	float ResultFloat = (Selector) ? (Angle) : (HeadHandAngleLimit * FMath::Sign(LastAngle));

	return FMath::Lerp(0.f, ResultFloat, HeadHandAlpha);
}

FVector UVRInverseKinematicsComponent::GetShoulderLocation()
{
	//To Settings_NeckOffset
	FVector Offset = NeckOffset;
	FRotator HeadRotator = Head_Local.Rotator();
	FRotator HeadYaw = FRotator(0.f, HeadRotator.Yaw, HeadRotator.Roll);

	Offset = HeadYaw.RotateVector(Offset);

	FRotator Delta = FRotator(HeadRotator.Pitch, 0.f, 0.f);

	FVector ShoulderTranslation = Head_Local.GetTranslation() + Offset;
	FTransform TempShoulder = FTransform(FRotator::ZeroRotator, ShoulderTranslation, FVector::OneVector);
	FTransform TempRotatedShoulder = RotatePointAroundPivot(TempShoulder, Head_Local, Delta);

	return TempRotatedShoulder.GetTranslation() + FVector(0.f, 0.f, -17.f);
}

FTransform UVRInverseKinematicsComponent::RotatePointAroundPivot(FTransform Point, FTransform Pivot, FRotator Delta)
{
	FTransform PointInPivotSpace = Point * Pivot.Inverse();
	FTransform RotatedInPivotSpace = PointInPivotSpace * FTransform(Delta, FVector::ZeroVector, FVector::OneVector);
	return RotatedInPivotSpace * Pivot;
}

void UVRInverseKinematicsComponent::SetUpperArms()
{
	// Left
	LeftUpperArm_Shoulder = RotateUpperArm(true, LeftHand_Shoulder.GetTranslation());
	const FVector& LeftX = (LeftUpperArm_Shoulder * Shoulder_World).GetTranslation() - Shoulder_World.GetTranslation();
	const FVector& LeftZ = FVector::UpVector;
	LeftClavicle_Local = FTransform(UKismetMathLibrary::MakeRotFromXZ(LeftX, LeftZ), FVector::ZeroVector, FVector::OneVector) * Origin_Local;

	// Right
	RightUpperArm_Shoulder = RotateUpperArm(false, RightHand_Shoulder.GetTranslation());
	const FVector& RightX = (RightUpperArm_Shoulder * Shoulder_World).GetTranslation() - Shoulder_World.GetTranslation();
	const FVector& RightZ = FVector::UpVector;
	RightClavicle_Local = FTransform(UKismetMathLibrary::MakeRotFromXZ(RightX, RightZ), FVector::ZeroVector, FVector::OneVector) * Origin_Local;
}

FTransform UVRInverseKinematicsComponent::RotateUpperArm(bool isLeft, const FVector& HandTranslation)
{
	const float Sign = (isLeft) ? (1.f) : (-1.f);

	const FVector InitialUpperArmTranslation = FVector::RightVector * UpperArmsDistance / (2.f * Sign);
	const FVector HandUpperArmDirection = HandTranslation - InitialUpperArmTranslation;

	const float ForwardDistanceRatio = FVector::DotProduct(HandUpperArmDirection, FVector::ForwardVector) / ArmLength;
	const float UpwardsDistanceRatio = FVector::DotProduct(HandUpperArmDirection, FVector::UpVector) / ArmLength;

	float Yaw;
	if (ForwardDistanceRatio > 0)
	{
		const float TempYaw = (ForwardDistanceRatio - 0.5f) * DistinctShoulderRotationMultiplier;
		Yaw = FMath::Clamp(TempYaw, 0.f, DistinctShoulderRotationLimit) + ClavicleOffset;
	}
	else
	{
		const float TempYaw = (ForwardDistanceRatio - 0.08f) * DistinctShoulderRotationMultiplier;
		Yaw = FMath::Clamp(TempYaw, -DistinctShoulderRotationLimit, 0.f) + ClavicleOffset;
	}

	const float TempRoll = (UpwardsDistanceRatio - 0.2f) * DistinctShoulderRotationMultiplier;
	const float Roll = FMath::Clamp(TempRoll, 0.f, DistinctShoulderRotationLimit);

	const float NotSign = (isLeft) ? (-1.f) : (1.f);
	const FRotator Rotation = FRotator(0.f, Yaw * NotSign, Roll * NotSign);

	return FTransform(Rotation, InitialUpperArmTranslation, FVector::OneVector).Inverse();
}

void UVRInverseKinematicsComponent::SolveArms()
{
	//LEFT ARM
	FVector HandLoc = LeftHand_Shoulder.GetTranslation();
	FRotator HandRot = LeftHand_Shoulder.GetRotation().Rotator();
	FVector UpperArmLoc = LeftUpperArm_Shoulder.GetTranslation();

	FTransform UpperArmBase = FTransform::Identity;
	FTransform LowerArmBase = FTransform::Identity;
	FTransform UpperArm = FTransform::Identity;
	FTransform LowerArm = FTransform::Identity;

	SetElbowBasePosition(UpperArmLoc, HandLoc, true, UpperArmBase, LowerArmBase);

	float BaseAngle = RotateElbowByHandPosition(HandLoc, true);

	RotateElbow(BaseAngle, UpperArmBase, LowerArmBase, HandLoc, true, UpperArm, LowerArm);

	float Angle = RotateElbowByHandRotation(LowerArm, HandRot);
	float SafeAngle = SafeguardAngle(LeftElbowHandAngle, Angle, 120.f);

	LeftElbowHandAngle = FMath::FInterpTo(LeftElbowHandAngle, SafeAngle, CachedDeltaTime, ElbowHandsRotSpeed);

	RotateElbow(LeftElbowHandAngle + BaseAngle, UpperArmBase, LowerArmBase, HandLoc, true, LeftUpperArm_Shoulder, LeftLowerArm_Shoulder);

	LeftUpperArm_World = LeftUpperArm_Shoulder * Shoulder_World;
	LeftLowerArm_World = LeftLowerArm_Shoulder * Shoulder_World;

	float Roll = FMath::Max((LeftHand_World.GetTranslation() - LeftUpperArm_World.GetTranslation()).Z, 0.f);
	FTransform UpperArmS = LeftUpperArm_World * Origin_Local;

	LeftUpperArm_Local = FTransform(UKismetMathLibrary::ComposeRotators(FRotator(0.f, 0.f, Roll), UpperArmS.Rotator()), UpperArmS.GetTranslation());
	LeftLowerArm_Local = LeftLowerArm_World * Origin_Local;

	//RIGHT ARM
	HandLoc = RightHand_Shoulder.GetTranslation();
	HandRot = RightHand_Shoulder.GetRotation().Rotator();
	UpperArmLoc = RightUpperArm_Shoulder.GetTranslation();

	SetElbowBasePosition(UpperArmLoc, HandLoc, false, UpperArmBase, LowerArmBase);

	BaseAngle = RotateElbowByHandPosition(HandLoc, false);

	UpperArm = FTransform::Identity;
	LowerArm = FTransform::Identity;

	RotateElbow(BaseAngle, UpperArmBase, LowerArmBase, HandLoc, false, UpperArm, LowerArm);

	Angle = RotateElbowByHandRotation(LowerArm, HandRot);

	SafeAngle = SafeguardAngle(RightElbowHandAngle, Angle, 120.f);

	RightElbowHandAngle = FMath::FInterpTo(RightElbowHandAngle, SafeAngle, CachedDeltaTime, ElbowHandsRotSpeed);

	RotateElbow(RightElbowHandAngle + BaseAngle, UpperArmBase, LowerArmBase, HandLoc, false, RightUpperArm_Shoulder, RightLowerArm_Shoulder);

	RightUpperArm_World = RightUpperArm_Shoulder * Shoulder_World;
	RightLowerArm_World = RightLowerArm_Shoulder * Shoulder_World;

	Roll = -1.f * FMath::Max((RightHand_World.GetTranslation() - RightUpperArm_World.GetTranslation()).Z, 0.f);
	UpperArmS = RightUpperArm_World * Origin_Local;

	RightUpperArm_Local = FTransform(UKismetMathLibrary::ComposeRotators(FRotator(0.f, 0.f, Roll), UpperArmS.Rotator()), UpperArmS.GetTranslation());
	RightLowerArm_Local = RightLowerArm_World * Origin_Local;
}

void UVRInverseKinematicsComponent::SetElbowBasePosition(const FVector& UpperArm, const FVector& Hand, bool IsLeft, FTransform& UpperArmTransform, FTransform& LowerArmTransform)
{
	const float UpperArmToHandlen = (UpperArm - Hand).Size();
	const float Sign = (IsLeft) ? (-1.f) : (1.f);
	const float Beta = CosineRule(UpperArmLength, UpperArmToHandlen, LowerArmLength) * Sign;
	float Omega = CosineRule(LowerArmLength, UpperArmLength, UpperArmToHandlen);
	Omega = (IsLeft) ? (180.f - Omega) : (180.f + Omega);

	const FRotator A = FQuat::FindBetweenNormals(FVector::ForwardVector, (Hand - UpperArm).GetSafeNormal()).Rotator();
	const FRotator B = FQuat(UKismetMathLibrary::GetUpVector(A), FMath::DegreesToRadians(Beta)).Rotator();
	UpperArmTransform = FTransform((UKismetMathLibrary::ComposeRotators(A, B)), UpperArm, FVector::OneVector);

	const FTransform TempLowerArm = FTransform(FRotator(0.f, Omega, 0.f), FVector::ForwardVector * UpperArmLength, FVector::OneVector);
	LowerArmTransform = TempLowerArm * UpperArmTransform;
}

void UVRInverseKinematicsComponent::RotateElbow(float Angle, const FTransform& UpperArm, const FTransform& LowerArm, const FVector& HandLoc, bool IsLeft, FTransform& NewUpperArm, FTransform& NewLowerArm)
{
	const FVector PivotLoc =
		UpperArm.GetTranslation() + UKismetMathLibrary::ProjectVectorOnToVector(UpperArm.GetTranslation() - LowerArm.GetTranslation(), UpperArm.GetTranslation() - HandLoc);

	const FVector Forward = UpperArm.GetTranslation() - HandLoc;
	const FVector Right = FVector::CrossProduct(UKismetMathLibrary::GetUpVector(UpperArm.Rotator()), Forward);
	const FVector Up = UKismetMathLibrary::GetUpVector(UpperArm.Rotator());
	FRotator PivotRot = UKismetMathLibrary::MakeRotationFromAxes(Forward, Right, Up);

	const float Roll = (IsLeft) ? (180.f - Angle) : (180.f + Angle);
	const FRotator Delta = FRotator(0.f, 0.f, Roll);
	const FTransform Pivot = FTransform(PivotRot, PivotLoc, FVector::OneVector);

	NewUpperArm = RotatePointAroundPivot(UpperArm, Pivot, Delta);
	NewLowerArm = RotatePointAroundPivot(LowerArm, Pivot, Delta);
}

float UVRInverseKinematicsComponent::RotateElbowByHandPosition(const FVector& Hand, bool IsLeft)
{
	const FVector TempHand = Hand / ArmLength;
	const float Sign = (IsLeft) ? (1.f) : (-1.f);
	const float Angle = ElbowBaseOffsetAngle +
		(ElbowYWeight * FMath::Max(0.f, (TempHand.Y * Sign) + ElbowYDistanceStart));
	return Angle;
}

float UVRInverseKinematicsComponent::RotateElbowByHandRotation(const FTransform& LowerArm, FRotator Hand)
{
	const FRotator LowerArmRot = LowerArm.Rotator();
	const FVector LowerArmRight = UKismetMathLibrary::GetRightVector(LowerArmRot);
	const FVector ProjectedForward = UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(Hand), LowerArmRight).GetSafeNormal();

	const FVector LowerArmForward = UKismetMathLibrary::GetForwardVector(LowerArmRot);
	float TempAngle = CosineRule(1.f, 1.f, (ProjectedForward - LowerArmForward).Size());

	const float Sign = (FVector::DotProduct(FVector::CrossProduct(ProjectedForward, LowerArmForward), LowerArmRight) < 0.f)	? (1.f) : (-1.f);
	TempAngle = TempAngle * Sign;
	TempAngle = TempAngle * ElbowRotFromHandRotAlpha;

	return TempAngle;
}

float UVRInverseKinematicsComponent::CosineRule(float Adjacent1, float Adjacent2, float Opposite)
{
	return FMath::RadiansToDegrees(FMath::Acos(((Adjacent1 * Adjacent1) + (Adjacent2 * Adjacent2) - (Opposite * Opposite)) / (Adjacent1 * Adjacent2 * 2.f)));
}

float UVRInverseKinematicsComponent::SafeguardAngle(float Last, float Current, float Threshold)
{
	return (FMath::Abs(Last - Current) > Threshold) ? (Last) : (Current);
}

void UVRInverseKinematicsComponent::UpdateIKData()
{
	UpperBodyIKData.Head = Head_Local.Rotator();
	UpperBodyIKData.Pelvis = Shoulder_Local.Rotator();

	//Left
	UpperBodyIKData.LeftClavicle = LeftClavicle_Local.Rotator();
	UpperBodyIKData.LeftUpperArm = LeftUpperArm_Local.Rotator();
	UpperBodyIKData.LeftLowerArm = LeftLowerArm_Local.Rotator();
	UpperBodyIKData.LeftHand = LeftHand_Local.Rotator();

	//Right
	UpperBodyIKData.RightClavicle = RightClavicle_Local.Rotator();
	UpperBodyIKData.RightUpperArm = RightUpperArm_Local.Rotator();
	UpperBodyIKData.RightLowerArm = RightLowerArm_Local.Rotator();
	UpperBodyIKData.RightHand = RightHand_Local.Rotator();
}

void UVRInverseKinematicsComponent::DrawDebug()
{
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(HeadEffector_World, 20, true, 15, FColor::Black);
	DrawDebugLine(GWorld, HeadEffector_World.GetLocation(), Shoulder_World.GetLocation(), FColor::White, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(Shoulder_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, Shoulder_World.GetLocation(), LeftUpperArm_World.GetLocation(), FColor::Red, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(LeftUpperArm_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, LeftUpperArm_World.GetLocation(), LeftLowerArm_World.GetLocation(), FColor::Yellow, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(LeftLowerArm_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, LeftLowerArm_World.GetLocation(), LeftHand_World.GetLocation(), FColor::Orange, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(LeftHand_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, Shoulder_World.GetLocation(), RightUpperArm_World.GetLocation(), FColor::Red, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(RightUpperArm_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, RightUpperArm_World.GetLocation(), RightLowerArm_World.GetLocation(), FColor::Yellow, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(RightLowerArm_World, 15, true, 12, FColor::White);
	
	DrawDebugLine(GWorld, RightLowerArm_World.GetLocation(), RightHand_World.GetLocation(), FColor::Orange, false, -1, 0, 4);
	UUtillityBlueprintFunctionLibrary::DrawDebugAxes(RightHand_World, 15, true, 12, FColor::White);
}
