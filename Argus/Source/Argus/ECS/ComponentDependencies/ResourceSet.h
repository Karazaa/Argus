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

	void Reset()
	{
		for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
		{
			m_resourceQuantities[i] = 0;
		}
	}

	FResourceSet operator-() const
	{
		FResourceSet outResources;
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			outResources.m_resourceQuantities[i] = -m_resourceQuantities[i];
		}

		return outResources;
	}

	FResourceSet operator-(const FResourceSet& right) const
	{
		FResourceSet outResources;
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			outResources.m_resourceQuantities[i] = m_resourceQuantities[i] - right.m_resourceQuantities[i];
		}

		return outResources;
	}

	friend bool operator>(const FResourceSet& left, const FResourceSet& right)
	{
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
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
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
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
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
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
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			if (left.m_resourceQuantities[i] > right.m_resourceQuantities[i])
			{
				return false;
			}
		}

		return false;
	}

	bool CanAffordResourceChange(const FResourceSet& otherResourceSetRepresentingChange) const
	{
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			if (otherResourceSetRepresentingChange.m_resourceQuantities[i] < 0 && 
				otherResourceSetRepresentingChange.m_resourceQuantities[i] < -m_resourceQuantities[i])
			{
				return false;
			}
		}
		return true;
	}

	void ApplyResourceChange(const FResourceSet& otherResourceSetRepresentingChange)
	{
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			m_resourceQuantities[i] += otherResourceSetRepresentingChange.m_resourceQuantities[i];
		}
	}

	FResourceSet CalculateResourceChangeAffordable(const FResourceSet& otherResourceSetRepresentingChange, const FResourceSet* maximumResources = nullptr) const
	{
		FResourceSet changeApplied;
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			const int32 potentialChange = m_resourceQuantities[i] + otherResourceSetRepresentingChange.m_resourceQuantities[i];
			if (maximumResources && potentialChange >= maximumResources->m_resourceQuantities[i])
			{
				changeApplied.m_resourceQuantities[i] = maximumResources->m_resourceQuantities[i] - m_resourceQuantities[i];
			}
			else if (potentialChange < 0)
			{
				changeApplied.m_resourceQuantities[i] = -m_resourceQuantities[i];
			}
			else
			{
				changeApplied.m_resourceQuantities[i] = otherResourceSetRepresentingChange.m_resourceQuantities[i];
			}
		}

		return changeApplied;
	}

	bool IsEmpty() const
	{
		const uint8 numResources = static_cast<uint8>(EResourceType::Count);
		for (uint8 i = 0u; i < numResources; ++i)
		{
			if (m_resourceQuantities[i] != 0)
			{
				return false;
			}
		}
		return true;
	}
};