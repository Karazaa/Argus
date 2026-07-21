// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/CombatInfo.h"
#include "ComponentDependencies/ResourceSet.h"

#include "EntityCategory.generated.h"

UENUM()
enum class EEntityCategoryType : uint8
{
	Extractor,
	Carrier,
	ResourceSink,
	ResourceSource,
	Combatant,
	Count
};

struct EntityCategory
{
	EEntityCategoryType m_entityCategoryType = EEntityCategoryType::Count;
	EResourceType m_resourceType = EResourceType::Count;
	ERangedAttackCapability m_attackCapability = ERangedAttackCapability::Count;

	bool operator==(const EntityCategory& other) const
	{
		return m_entityCategoryType == other.m_entityCategoryType && m_resourceType == other.m_resourceType && m_attackCapability == other.m_attackCapability;
	}
};
FORCEINLINE uint32 GetTypeHash(const EntityCategory& entityCategory)
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