// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRSystemDataType.generated.h"

/*
 * ���� �ۼ��� : ���¿�
 * ���� ������ : �̿뼱
 * ���� ������ : 2023.03.16
 * ���۷��� : VRExpansionPlugin, Project_One
 *
*/

UENUM()
enum class VRSystemVectorQuantization : uint8
{
	// ������ Vector Component�� �Ҽ��� ���� ���ڸ��� �����ϸ鼭 �ݿø�
	RoundOneDecimal = 0,
	// ������ Vector Component�� �Ҽ��� ���� ���ڸ��� �����ϸ鼭 �ݿø� 
	RoundTwoDecimals = 1
};

UENUM()
enum class VRSystemRotationQuantization : uint8
{
	// ������ Rotation Component�� 10bits(1024��)�� �ݿø� 
	RoundTo10Bits = 0,
	// ������ Rotation Component�� Short�� �ݿø� 
	RoundToShort = 1
};

USTRUCT()
struct PROJECT_TWO_API FVRSystemComponentRepPosition
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(Transient)
	FVector Position;

	UPROPERTY(Transient)
	FRotator Rotation;

	// Vector Components�� ����� Quatization Level
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	VRSystemVectorQuantization QuantizationLevel;

	// Rotation Components�� ����� Quatization Level
	// 10bits ��� ���� Replication �� �� 2.25 byte�� ����
	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
	VRSystemRotationQuantization RotationQuantizationLevel;

	FORCEINLINE uint16 CompressAxisTo10BitShort(float Angle)
	{
		// [0->360) �� [0->1024)�� ����(Map)�ϰ� Mask Off any Widing
		return FMath::RoundToInt(Angle * 1024.f / 360.f) & 0xFFFF;
	}

	FORCEINLINE float DecompressAxisFrom10BitShort(uint16 Angle)
	{
		// [0->1024) �� [0->360)���� ����(Map)
		return (Angle * 360.f / 1024.f);
	}

	FVRSystemComponentRepPosition() :
		QuantizationLevel(VRSystemVectorQuantization::RoundTwoDecimals),
		RotationQuantizationLevel(VRSystemRotationQuantization::RoundToShort)
	{
		Position = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
	}

	/* Network serialization */
	// Doing a custom NetSerialize here because this is sent via RPCs and should change on every update
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		// Defines the level of Quantization
		Ar.SerializeBits(&QuantizationLevel, 1); // Only two values 0:1
		Ar.SerializeBits(&RotationQuantizationLevel, 1); // Only two values 0:1

		uint16 ShortPitch = 0;
		uint16 ShortYaw = 0;
		uint16 ShortRoll = 0;

		/**
		*	Valid range 100: 2^22 / 100 = +/- 41,943.04 (419.43 meters)
		*	Valid range 10: 2^18 / 10 = +/- 26,214.4 (262.144 meters)
		*	Pos rep is assumed to be in relative space for a tracked component, these numbers should be fine
		*/
		if (Ar.IsSaving())
		{
			switch (QuantizationLevel)
			{
			case VRSystemVectorQuantization::RoundTwoDecimals: bOutSuccess &= SerializePackedVector<100, 22/*30*/>(Position, Ar); break;
			case VRSystemVectorQuantization::RoundOneDecimal: bOutSuccess &= SerializePackedVector<10, 18/*24*/>(Position, Ar); break;
			}

			switch (RotationQuantizationLevel)
			{
			case VRSystemRotationQuantization::RoundTo10Bits:
			{
				ShortPitch = CompressAxisTo10BitShort(Rotation.Pitch);
				ShortYaw = CompressAxisTo10BitShort(Rotation.Yaw);
				ShortRoll = CompressAxisTo10BitShort(Rotation.Roll);

				Ar.SerializeBits(&ShortPitch, 10);
				Ar.SerializeBits(&ShortYaw, 10);
				Ar.SerializeBits(&ShortRoll, 10);
			}break;

			case VRSystemRotationQuantization::RoundToShort:
			{
				ShortPitch = FRotator::CompressAxisToShort(Rotation.Pitch);
				ShortYaw = FRotator::CompressAxisToShort(Rotation.Yaw);
				ShortRoll = FRotator::CompressAxisToShort(Rotation.Roll);

				Ar << ShortPitch;
				Ar << ShortYaw;
				Ar << ShortRoll;
			}break;
			}
		}
		else // If loading
		{
			switch (QuantizationLevel)
			{
			case VRSystemVectorQuantization::RoundTwoDecimals: bOutSuccess &= SerializePackedVector<100, 22/*30*/>(Position, Ar); break;
			case VRSystemVectorQuantization::RoundOneDecimal: bOutSuccess &= SerializePackedVector<10, 18/*24*/>(Position, Ar); break;
			}

			switch (RotationQuantizationLevel)
			{
			case VRSystemRotationQuantization::RoundTo10Bits:
			{
				Ar.SerializeBits(&ShortPitch, 10);
				Ar.SerializeBits(&ShortYaw, 10);
				Ar.SerializeBits(&ShortRoll, 10);

				Rotation.Pitch = DecompressAxisFrom10BitShort(ShortPitch);
				Rotation.Yaw = DecompressAxisFrom10BitShort(ShortYaw);
				Rotation.Roll = DecompressAxisFrom10BitShort(ShortRoll);
			}break;

			case VRSystemRotationQuantization::RoundToShort:
			{
				Ar << ShortPitch;
				Ar << ShortYaw;
				Ar << ShortRoll;

				Rotation.Pitch = FRotator::DecompressAxisFromShort(ShortPitch);
				Rotation.Yaw = FRotator::DecompressAxisFromShort(ShortYaw);
				Rotation.Roll = FRotator::DecompressAxisFromShort(ShortRoll);
			}break;
			}
		}

		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits< FVRSystemComponentRepPosition > : public TStructOpsTypeTraitsBase2<FVRSystemComponentRepPosition>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};