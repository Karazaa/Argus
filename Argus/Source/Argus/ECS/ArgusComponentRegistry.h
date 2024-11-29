// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include <bitset>
#include <unordered_map>

// Begin component specific includes.
#include "ComponentDefinitions\AbilityComponent.h"
#include "ComponentDefinitions\HealthComponent.h"
#include "ComponentDefinitions\IdentityComponent.h"
#include "ComponentDefinitions\NavigationComponent.h"
#include "ComponentDefinitions\SpawningComponent.h"
#include "ComponentDefinitions\TargetingComponent.h"
#include "ComponentDefinitions\TaskComponent.h"
#include "ComponentDefinitions\TransformComponent.h"

// Begin dynamically allocated component specific includes.
#include "DynamicAllocComponentDefinitions\SpatialPartitioningComponent.h"

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

	static void RemoveComponentsForEntity(uint16 entityId);
	static void FlushAllComponents();
	static void AppendComponentDebugStrings(uint16 entityId, FString& debugStringToAppendTo);

	static constexpr uint32 k_numComponentTypes = 9;

	// Begin component specific template specifiers.
	
#pragma region AbilityComponent
private:
	static AbilityComponent s_AbilityComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isAbilityComponentActive;
public:
	template<>
	inline AbilityComponent* GetComponent<AbilityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(AbilityComponent));
			return nullptr;
		}

		if (!s_isAbilityComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_AbilityComponents[entityId];
	}

	template<>
	inline AbilityComponent* AddComponent<AbilityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(AbilityComponent));
			return nullptr;
		}

		if (s_isAbilityComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AbilityComponent), entityId);
			return &s_AbilityComponents[entityId];
		}

		s_AbilityComponents[entityId] = AbilityComponent();
		s_isAbilityComponentActive.set(entityId);
		return &s_AbilityComponents[entityId];
	}

	template<>
	inline AbilityComponent* GetOrAddComponent<AbilityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(AbilityComponent));
			return nullptr;
		}

		if (s_isAbilityComponentActive[entityId])
		{
			return &s_AbilityComponents[entityId];
		}
		else
		{
			s_AbilityComponents[entityId] = AbilityComponent();
			s_isAbilityComponentActive.set(entityId);
			return &s_AbilityComponents[entityId];
		}
	}
#pragma endregion
#pragma region HealthComponent
private:
	static HealthComponent s_HealthComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isHealthComponentActive;
public:
	template<>
	inline HealthComponent* GetComponent<HealthComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
			return nullptr;
		}

		if (s_isHealthComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), entityId);
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(HealthComponent));
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
#pragma endregion
#pragma region IdentityComponent
private:
	static IdentityComponent s_IdentityComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isIdentityComponentActive;
public:
	template<>
	inline IdentityComponent* GetComponent<IdentityComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
			return nullptr;
		}

		if (s_isIdentityComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), entityId);
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(IdentityComponent));
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
#pragma endregion
#pragma region NavigationComponent
private:
	static NavigationComponent s_NavigationComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isNavigationComponentActive;
public:
	template<>
	inline NavigationComponent* GetComponent<NavigationComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(NavigationComponent));
			return nullptr;
		}

		if (!s_isNavigationComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_NavigationComponents[entityId];
	}

	template<>
	inline NavigationComponent* AddComponent<NavigationComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(NavigationComponent));
			return nullptr;
		}

		if (s_isNavigationComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), entityId);
			return &s_NavigationComponents[entityId];
		}

		s_NavigationComponents[entityId] = NavigationComponent();
		s_isNavigationComponentActive.set(entityId);
		return &s_NavigationComponents[entityId];
	}

	template<>
	inline NavigationComponent* GetOrAddComponent<NavigationComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(NavigationComponent));
			return nullptr;
		}

		if (s_isNavigationComponentActive[entityId])
		{
			return &s_NavigationComponents[entityId];
		}
		else
		{
			s_NavigationComponents[entityId] = NavigationComponent();
			s_isNavigationComponentActive.set(entityId);
			return &s_NavigationComponents[entityId];
		}
	}
#pragma endregion
#pragma region SpawningComponent
private:
	static SpawningComponent s_SpawningComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isSpawningComponentActive;
public:
	template<>
	inline SpawningComponent* GetComponent<SpawningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpawningComponent));
			return nullptr;
		}

		if (!s_isSpawningComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_SpawningComponents[entityId];
	}

	template<>
	inline SpawningComponent* AddComponent<SpawningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpawningComponent));
			return nullptr;
		}

		if (s_isSpawningComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawningComponent), entityId);
			return &s_SpawningComponents[entityId];
		}

		s_SpawningComponents[entityId] = SpawningComponent();
		s_isSpawningComponentActive.set(entityId);
		return &s_SpawningComponents[entityId];
	}

	template<>
	inline SpawningComponent* GetOrAddComponent<SpawningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpawningComponent));
			return nullptr;
		}

		if (s_isSpawningComponentActive[entityId])
		{
			return &s_SpawningComponents[entityId];
		}
		else
		{
			s_SpawningComponents[entityId] = SpawningComponent();
			s_isSpawningComponentActive.set(entityId);
			return &s_SpawningComponents[entityId];
		}
	}
#pragma endregion
#pragma region TargetingComponent
private:
	static TargetingComponent s_TargetingComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isTargetingComponentActive;
public:
	template<>
	inline TargetingComponent* GetComponent<TargetingComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
			return nullptr;
		}

		if (s_isTargetingComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent), entityId);
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TargetingComponent));
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
#pragma endregion
#pragma region TaskComponent
private:
	static TaskComponent s_TaskComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isTaskComponentActive;
public:
	template<>
	inline TaskComponent* GetComponent<TaskComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TaskComponent));
			return nullptr;
		}

		if (!s_isTaskComponentActive[entityId])
		{
			return nullptr;
		}

		return &s_TaskComponents[entityId];
	}

	template<>
	inline TaskComponent* AddComponent<TaskComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TaskComponent));
			return nullptr;
		}

		if (s_isTaskComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TaskComponent), entityId);
			return &s_TaskComponents[entityId];
		}

		s_TaskComponents[entityId] = TaskComponent();
		s_isTaskComponentActive.set(entityId);
		return &s_TaskComponents[entityId];
	}

	template<>
	inline TaskComponent* GetOrAddComponent<TaskComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TaskComponent));
			return nullptr;
		}

		if (s_isTaskComponentActive[entityId])
		{
			return &s_TaskComponents[entityId];
		}
		else
		{
			s_TaskComponents[entityId] = TaskComponent();
			s_isTaskComponentActive.set(entityId);
			return &s_TaskComponents[entityId];
		}
	}
#pragma endregion
#pragma region TransformComponent
private:
	static TransformComponent s_TransformComponents[ArgusECSConstants::k_maxEntities];
	static std::bitset<ArgusECSConstants::k_maxEntities> s_isTransformComponentActive;
public:
	template<>
	inline TransformComponent* GetComponent<TransformComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
			return nullptr;
		}

		if (s_isTransformComponentActive[entityId])
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), entityId);
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
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(TransformComponent));
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
#pragma endregion
	
	// Begin dynamically allocated component specific template specifiers.

#pragma region SpatialPartitioningComponent
private:
	static std::unordered_map<uint16, SpatialPartitioningComponent> s_SpatialPartitioningComponents;
public:
	template<>
	inline SpatialPartitioningComponent* GetComponent<SpatialPartitioningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when getting %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpatialPartitioningComponent));
			return nullptr;
		}

		if (!s_SpatialPartitioningComponents.contains(entityId))
		{
			return nullptr;
		}

		return &s_SpatialPartitioningComponents[entityId];
	}

	template<>
	inline SpatialPartitioningComponent* AddComponent<SpatialPartitioningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpatialPartitioningComponent));
			return nullptr;
		}

		if (s_SpatialPartitioningComponents.contains(entityId))
		{
			ARGUS_LOG(ArgusECSLog, Warning, TEXT("[%s] Attempting to add a %s to entity %d, which already has one."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent), entityId);
			return &s_SpatialPartitioningComponents[entityId];
		}

		s_SpatialPartitioningComponents[entityId] = SpatialPartitioningComponent();
		return &s_SpatialPartitioningComponents[entityId];
	}

	template<>
	inline SpatialPartitioningComponent* GetOrAddComponent<SpatialPartitioningComponent>(uint16 entityId)
	{
		if (entityId >= ArgusECSConstants::k_maxEntities)
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d, used when adding %s."), ARGUS_FUNCNAME, entityId, ARGUS_NAMEOF(SpatialPartitioningComponent));
			return nullptr;
		}

		if (!s_SpatialPartitioningComponents.contains(entityId))
		{
			s_SpatialPartitioningComponents[entityId] = SpatialPartitioningComponent();
		}
		return &s_SpatialPartitioningComponents[entityId];
	}
#pragma endregion
};
