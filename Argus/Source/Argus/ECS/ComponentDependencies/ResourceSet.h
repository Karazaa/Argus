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
	int32 m_resourceQuantities[(uint8)EResourceType::Count] = { 0, 0, 0 };

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
};