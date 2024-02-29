// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusUtil.h"
#include "ComponentDefinitions/ArgusComponentDefinitions.h"
#include <bitset>

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

	template<>
	inline HealthComponent* GetComponent<HealthComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[ECS] Invalid entity id %d, used when getting HealthComponent."), entityId);
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
			UE_LOG(ArgusGameLog, Error, TEXT("[ECS] Invalid entity id %d, used when adding HealthComponent."), entityId);
			return nullptr;
		}

		if (s_isHealthComponentActive[entityId])
		{
			UE_LOG(ArgusGameLog, Warning, TEXT("[ECS] Attempting to add a HealthComponent to entity %d, which already has one."), entityId);
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