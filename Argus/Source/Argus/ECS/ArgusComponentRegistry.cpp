// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ArgusComponentRegistry.h"

// Begin component specific definitions
#pragma region AbilityComponent
AbilityComponent ArgusComponentRegistry::s_AbilityComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isAbilityComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region CombatComponent
CombatComponent ArgusComponentRegistry::s_CombatComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isCombatComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region ConstructionComponent
ConstructionComponent ArgusComponentRegistry::s_ConstructionComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isConstructionComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region HealthComponent
HealthComponent ArgusComponentRegistry::s_HealthComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isHealthComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region IdentityComponent
IdentityComponent ArgusComponentRegistry::s_IdentityComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isIdentityComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region NavigationComponent
NavigationComponent ArgusComponentRegistry::s_NavigationComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isNavigationComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region ObserversComponent
ObserversComponent ArgusComponentRegistry::s_ObserversComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isObserversComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region SpawningComponent
SpawningComponent ArgusComponentRegistry::s_SpawningComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isSpawningComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TargetingComponent
TargetingComponent ArgusComponentRegistry::s_TargetingComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTargetingComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TaskComponent
TaskComponent ArgusComponentRegistry::s_TaskComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTaskComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TimerComponent
TimerComponent ArgusComponentRegistry::s_TimerComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTimerComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TransformComponent
TransformComponent ArgusComponentRegistry::s_TransformComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTransformComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region InputInterfaceComponent
std::unordered_map<uint16, InputInterfaceComponent> ArgusComponentRegistry::s_InputInterfaceComponents;
#pragma endregion
#pragma region ResourceComponent
std::unordered_map<uint16, ResourceComponent> ArgusComponentRegistry::s_ResourceComponents;
#pragma endregion
#pragma region ReticleComponent
std::unordered_map<uint16, ReticleComponent> ArgusComponentRegistry::s_ReticleComponents;
#pragma endregion
#pragma region SpatialPartitioningComponent
std::unordered_map<uint16, SpatialPartitioningComponent> ArgusComponentRegistry::s_SpatialPartitioningComponents;
#pragma endregion

void ArgusComponentRegistry::RemoveComponentsForEntity(uint16 entityId)
{
	if (entityId >= ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid entity id %d."), ARGUS_FUNCNAME, entityId);
		return;
	}

	// Begin set bitset bits to false
	s_isAbilityComponentActive.set(entityId, false);
	s_isCombatComponentActive.set(entityId, false);
	s_isConstructionComponentActive.set(entityId, false);
	s_isHealthComponentActive.set(entityId, false);
	s_isIdentityComponentActive.set(entityId, false);
	s_isNavigationComponentActive.set(entityId, false);
	s_isObserversComponentActive.set(entityId, false);
	s_isSpawningComponentActive.set(entityId, false);
	s_isTargetingComponentActive.set(entityId, false);
	s_isTaskComponentActive.set(entityId, false);
	s_isTimerComponentActive.set(entityId, false);
	s_isTransformComponentActive.set(entityId, false);

	// Begin set component values
	s_AbilityComponents[entityId] = AbilityComponent();
	s_CombatComponents[entityId] = CombatComponent();
	s_ConstructionComponents[entityId] = ConstructionComponent();
	s_HealthComponents[entityId] = HealthComponent();
	s_IdentityComponents[entityId] = IdentityComponent();
	s_NavigationComponents[entityId] = NavigationComponent();
	s_ObserversComponents[entityId] = ObserversComponent();
	s_SpawningComponents[entityId] = SpawningComponent();
	s_TargetingComponents[entityId] = TargetingComponent();
	s_TaskComponents[entityId] = TaskComponent();
	s_TimerComponents[entityId] = TimerComponent();
	s_TransformComponents[entityId] = TransformComponent();

	// Begin remove dynamically allocated components
	if (s_InputInterfaceComponents.contains(entityId))
	{
		s_InputInterfaceComponents.erase(entityId);
	}
	if (s_ResourceComponents.contains(entityId))
	{
		s_ResourceComponents.erase(entityId);
	}
	if (s_ReticleComponents.contains(entityId))
	{
		s_ReticleComponents.erase(entityId);
	}
	if (s_SpatialPartitioningComponents.contains(entityId))
	{
		s_SpatialPartitioningComponents.erase(entityId);
	}
}

void ArgusComponentRegistry::FlushAllComponents()
{
	// Begin flush active component bitsets
	s_isAbilityComponentActive.reset();
	s_isCombatComponentActive.reset();
	s_isConstructionComponentActive.reset();
	s_isHealthComponentActive.reset();
	s_isIdentityComponentActive.reset();
	s_isNavigationComponentActive.reset();
	s_isObserversComponentActive.reset();
	s_isSpawningComponentActive.reset();
	s_isTargetingComponentActive.reset();
	s_isTaskComponentActive.reset();
	s_isTimerComponentActive.reset();
	s_isTransformComponentActive.reset();

	// Begin flush component values
	for (uint16 i = 0u; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_AbilityComponents[i] = AbilityComponent();
		s_CombatComponents[i] = CombatComponent();
		s_ConstructionComponents[i] = ConstructionComponent();
		s_HealthComponents[i] = HealthComponent();
		s_IdentityComponents[i] = IdentityComponent();
		s_NavigationComponents[i] = NavigationComponent();
		s_ObserversComponents[i] = ObserversComponent();
		s_SpawningComponents[i] = SpawningComponent();
		s_TargetingComponents[i] = TargetingComponent();
		s_TaskComponents[i] = TaskComponent();
		s_TimerComponents[i] = TimerComponent();
		s_TransformComponents[i] = TransformComponent();
	}

	// Begin flush dynamically allocated components
	s_InputInterfaceComponents.clear();
	s_ResourceComponents.clear();
	s_ReticleComponents.clear();
	s_SpatialPartitioningComponents.clear();
}

void ArgusComponentRegistry::AppendComponentDebugStrings(uint16 entityId, FString& debugStringToAppendTo)
{
	if (const AbilityComponent* AbilityComponentPtr = GetComponent<AbilityComponent>(entityId))
	{
		AbilityComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const CombatComponent* CombatComponentPtr = GetComponent<CombatComponent>(entityId))
	{
		CombatComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const ConstructionComponent* ConstructionComponentPtr = GetComponent<ConstructionComponent>(entityId))
	{
		ConstructionComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const HealthComponent* HealthComponentPtr = GetComponent<HealthComponent>(entityId))
	{
		HealthComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const IdentityComponent* IdentityComponentPtr = GetComponent<IdentityComponent>(entityId))
	{
		IdentityComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const NavigationComponent* NavigationComponentPtr = GetComponent<NavigationComponent>(entityId))
	{
		NavigationComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const ObserversComponent* ObserversComponentPtr = GetComponent<ObserversComponent>(entityId))
	{
		ObserversComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const SpawningComponent* SpawningComponentPtr = GetComponent<SpawningComponent>(entityId))
	{
		SpawningComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const TargetingComponent* TargetingComponentPtr = GetComponent<TargetingComponent>(entityId))
	{
		TargetingComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const TaskComponent* TaskComponentPtr = GetComponent<TaskComponent>(entityId))
	{
		TaskComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const TimerComponent* TimerComponentPtr = GetComponent<TimerComponent>(entityId))
	{
		TimerComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const TransformComponent* TransformComponentPtr = GetComponent<TransformComponent>(entityId))
	{
		TransformComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const InputInterfaceComponent* InputInterfaceComponentPtr = GetComponent<InputInterfaceComponent>(entityId))
	{
		InputInterfaceComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const ResourceComponent* ResourceComponentPtr = GetComponent<ResourceComponent>(entityId))
	{
		ResourceComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const ReticleComponent* ReticleComponentPtr = GetComponent<ReticleComponent>(entityId))
	{
		ReticleComponentPtr->GetDebugString(debugStringToAppendTo);
	}
	if (const SpatialPartitioningComponent* SpatialPartitioningComponentPtr = GetComponent<SpatialPartitioningComponent>(entityId))
	{
		SpatialPartitioningComponentPtr->GetDebugString(debugStringToAppendTo);
	}
}
