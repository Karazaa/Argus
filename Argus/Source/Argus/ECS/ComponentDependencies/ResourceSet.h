// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include "ResourceSet.generated.h"

UENUM(BlueprintType)
enum class EResourceType : uint8
{
	ResourceA,
	ResourceB,
	ResourceC,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FResourceSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 m_resourceQuantities[static_cast<uint8>(EResourceType::Count)] = { 0, 0, 0 };

	FResourceSet operator-() const;
	FResourceSet operator-(const FResourceSet& right) const;

	void Reset();
	bool CanAffordResourceChange(const FResourceSet& otherResourceSetRepresentingChange) const;
	void ApplyResourceChange(const FResourceSet& otherResourceSetRepresentingChange);
	bool IsEntirelyAtCap(const FResourceSet& capacityResrouceSet) const;
	FResourceSet MaskResourceSet(const FResourceSet& maskSet) const;
	FResourceSet CalculateResourceChangeAffordable(const FResourceSet& otherResourceSetRepresentingChange, const FResourceSet* maximumResources = nullptr) const;
	bool IsEmpty() const;

	friend bool operator>(const FResourceSet& left, const FResourceSet& right)
	{
		for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
		{
			if (left.m_resourceQuantities[i] > right.m_resourceQuantities[i])
			{
				return true;
			}
		}

		return false;
	}

	friend bool operator>=(const FResourceSet& left, const FResourceSet& right)
	{
		for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
		{
			if (left.m_resourceQuantities[i] >= right.m_resourceQuantities[i])
			{
				return true;
			}
		}

		return false;
	}

	friend bool operator<(const FResourceSet& left, const FResourceSet& right)
	{
		for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
		{
			if (left.m_resourceQuantities[i] >= right.m_resourceQuantities[i])
			{
				return false;
			}
		}

		return true;
	}

	friend bool operator<=(const FResourceSet& left, const FResourceSet& right)
	{
		for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
		{
			if (left.m_resourceQuantities[i] > right.m_resourceQuantities[i])
			{
				return false;
			}
		}

		return true;
	}
};