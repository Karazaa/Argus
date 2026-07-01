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
	EAttackType m_attackType = EAttackType::Melee;
	ERangedAttackCapability m_rangedCapability = ERangedAttackCapability::GroundedAndFlying;
};

namespace EntityCategoryUtils
{
	inline bool IsSpawnable(EEntityCategoryType type)
	{
		return	type != EEntityCategoryType::ResourceSink &&
				type != EEntityCategoryType::ResourceSource;
	}
}