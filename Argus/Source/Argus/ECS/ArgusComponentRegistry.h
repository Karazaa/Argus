// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusUtil.h"
#include <bitset>

// Begin component specific includes.
#include "ComponentDefinitions\HealthComponent.h"
#include "ComponentDefinitions\IdentityComponent.h"
#include "ComponentDefinitions\TargetingComponent.h"
#include "ComponentDefinitions\TransformComponent.h"

class ArgusComponentRegistry
{
public:
	template<typename ArgusComponent>
	static ArgusComponent* GetComponent(uint16 entityId)
	{
		return nullptr;
	}

	template<typename ArgusComponent>
	static ArgusComponent* AddComponent(uint16 entityId)
	{
		return nullptr;
	}

	template<typename ArgusComponent>
	static ArgusComponent* GetOrAddComponent(uint16 entityId)
	{
		return nullptr;
	}

	static void FlushAllComponents();

	static constexpr uint32 k_numComponentTypes = 4;

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

	template<>
	inline HealthComponent* GetOrAddComponent<HealthComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
			return nullptr;
		}

		if (s_isHealthComponentActive[entityId])
		{
			return &s_HealthComponents[entityId];
		}
		else
		{
			s_HealthComponents[entityId] = HealthComponent();
			s_isHealthComponentActive.set(entityId);
			return &s_HealthComponents[entityId];
		}
	}
// IdentityComponent ======================================================================================================
private:
	static IdentityComponent s_IdentityComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isIdentityComponentActive;
public:
	template<>
	inline IdentityComponent* GetComponent<IdentityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
			return nullptr;
		}

		if (!s_isIdentityComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_IdentityComponents[entityId];
	}

	template<>
	inline IdentityComponent* AddComponent<IdentityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
			return nullptr;
		}

		if (s_isIdentityComponentActive[entityId])
		{
			UE_LOG(ArgusGameLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), entityId);
			return &s_IdentityComponents[entityId];
		}

		s_IdentityComponents[entityId] = IdentityComponent();
		s_isIdentityComponentActive.set(entityId);
		return &s_IdentityComponents[entityId];
	}

	template<>
	inline IdentityComponent* GetOrAddComponent<IdentityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
			return nullptr;
		}

		if (s_isIdentityComponentActive[entityId])
		{
			return &s_IdentityComponents[entityId];
		}
		else
		{
			s_IdentityComponents[entityId] = IdentityComponent();
			s_isIdentityComponentActive.set(entityId);
			return &s_IdentityComponents[entityId];
		}
	}
// TargetingComponent ======================================================================================================
private:
	static TargetingComponent s_TargetingComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isTargetingComponentActive;
public:
	template<>
	inline TargetingComponent* GetComponent<TargetingComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
			return nullptr;
		}

		if (!s_isTargetingComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_TargetingComponents[entityId];
	}

	template<>
	inline TargetingComponent* AddComponent<TargetingComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
			return nullptr;
		}

		if (s_isTargetingComponentActive[entityId])
		{
			UE_LOG(ArgusGameLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent), entityId);
			return &s_TargetingComponents[entityId];
		}

		s_TargetingComponents[entityId] = TargetingComponent();
		s_isTargetingComponentActive.set(entityId);
		return &s_TargetingComponents[entityId];
	}

	template<>
	inline TargetingComponent* GetOrAddComponent<TargetingComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
			return nullptr;
		}

		if (s_isTargetingComponentActive[entityId])
		{
			return &s_TargetingComponents[entityId];
		}
		else
		{
			s_TargetingComponents[entityId] = TargetingComponent();
			s_isTargetingComponentActive.set(entityId);
			return &s_TargetingComponents[entityId];
		}
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

	template<>
	inline TransformComponent* GetOrAddComponent<TransformComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
			return nullptr;
		}

		if (s_isTransformComponentActive[entityId])
		{
			return &s_TransformComponents[entityId];
		}
		else
		{
			s_TransformComponents[entityId] = TransformComponent();
			s_isTransformComponentActive.set(entityId);
			return &s_TransformComponents[entityId];
		}
	}
};
