// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/CombatInfo.h"
#include "ComponentDependencies/ResourceSet.h"

#include "EntityCategory.generated.h"

UENUM(BlueprintType)
enum class EEntityCategoryType : uint8
{
	Extractor,
	Carrier,
	ResourceSink,
	ResourceSource,
	Combatant,
	Count
};

USTRUCT()
struct FEntityCategory
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	EEntityCategoryType m_entityCategoryType = EEntityCategoryType::Count;

	UPROPERTY(VisibleAnywhere)
	EResourceType m_resourceType = EResourceType::Count;

	UPROPERTY(VisibleAnywhere)
	ERangedAttackCapability m_attackCapability = ERangedAttackCapability::Count;

	FEntityCategory() = default;
	FEntityCategory(EEntityCategoryType entityCategoryType, EResourceType resourceType, ERangedAttackCapability attackCapability) :	m_entityCategoryType(m_entityCategoryType), 
																																	m_resourceType(m_resourceType), 
																																	m_attackCapability(m_attackCapability) {}

	bool operator==(const FEntityCategory& other) const
	{
		return m_entityCategoryType == other.m_entityCategoryType && m_resourceType == other.m_resourceType && m_attackCapability == other.m_attackCapability;
	}
};
FORCEINLINE uint32 GetTypeHash(const FEntityCategory& entityCategory)
{
	return HashCombine(GetTypeHash(entityCategory.m_entityCategoryType), GetTypeHash(entityCategory.m_resourceType), GetTypeHash(entityCategory.m_attackCapability));
}

namespace EntityCategoryUtils
{
	inline bool IsSpawnable(EEntityCategoryType type)
	{
		return	type != EEntityCategoryType::ResourceSink &&
				type != EEntityCategoryType::ResourceSource;
	}
}