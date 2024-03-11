// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusUtil.h"
#include <bitset>

// Begin component specific includes.
#include "ComponentDefinitions\HealthComponent.h"
#include "ComponentDefinitions\TransformComponent.h"

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
private:
	static HealthComponent s_HealthComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isHealthComponentActive;
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

		return &s_HealthComponents[entityId];
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
			return &s_HealthComponents[entityId];
		}

		s_HealthComponents[entityId] = HealthComponent();
		s_isHealthComponentActive.set(entityId);
		return &s_HealthComponents[entityId];
	}
// TransformComponent ======================================================================================================
private:
	static TransformComponent s_TransformComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isTransformComponentActive;
public:
	template<>
	inline TransformComponent* GetComponent<TransformComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
			return nullptr;
		}

		if (!s_isTransformComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_TransformComponents[entityId];
	}

	template<>
	inline TransformComponent* AddComponent<TransformComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
			return nullptr;
		}

		if (s_isTransformComponentActive[entityId])
		{
			UE_LOG(ArgusGameLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), entityId);
			return &s_TransformComponents[entityId];
		}

		s_TransformComponents[entityId] = TransformComponent();
		s_isTransformComponentActive.set(entityId);
		return &s_TransformComponents[entityId];
	}
};
