// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusUtil.h"
#include <bitset>

#include "ComponentDefinitions/HealthComponent.h"

class ArgusComponentRegistry
{
public:
	template<class ArgusComponent>
	static ArgusComponent* GetComponent(uint16 entityId)
	{
		return nullptr;
	}

	template<class ArgusComponent>
	static ArgusComponent* AddComponent(uint16 entityId)
	{
		return nullptr;
	}

	static void FlushAllComponents();

	// Begin component specific template specifiers.

// HealthComponent ======================================================================================================
public:
	template<>
	inline HealthComponent* GetComponent<HealthComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
			return nullptr;
		}

		if (!s_isHealthComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_healthComponents[entityId];
	}

	template<>
	inline HealthComponent* AddComponent<HealthComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
			return nullptr;
		}

		if (s_isHealthComponentActive[entityId])
		{
			UE_LOG(ArgusGameLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), entityId);
			return &s_healthComponents[entityId];
		}

		s_healthComponents[entityId].Reset();
		s_isHealthComponentActive.set(entityId);
		return &s_healthComponents[entityId];
	}

private:
	static HealthComponent									s_healthComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities>	s_isHealthComponentActive;
};