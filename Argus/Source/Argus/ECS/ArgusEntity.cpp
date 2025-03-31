// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const ArgusEntity ArgusEntity::k_emptyEntity = ArgusEntity();
std::bitset<ArgusECSConstants::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusECSConstants::k_maxEntities>();

uint16 ArgusEntity::s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
uint16 ArgusEntity::s_highestTakenEntityId = 0u;

void ArgusEntity::FlushAllEntities()
{
	ArgusComponentRegistry::FlushAllComponents();
	s_takenEntityIds.reset();
	s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
	s_highestTakenEntityId = 0u;
}

bool ArgusEntity::DoesEntityExist(uint16 id)
{
	if (id >= ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	return s_takenEntityIds[id];
}

bool ArgusEntity::IsReservedEntityId(uint16 id)
{
	uint16 sizeOfTeamEnum = sizeof(ETeam) * 8;
	return id >= (ArgusECSConstants::k_singletonEntityId - sizeOfTeamEnum);
}

ArgusEntity ArgusEntity::CreateEntity(uint16 lowestId)
{
	const uint16 id = GetNextLowestUntakenId(lowestId);

	if (id == ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an invalid ID value."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return k_emptyEntity;
	}

	if (s_takenEntityIds[id])
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an ID value of an already existing %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity));
		return k_emptyEntity;
	}

	s_takenEntityIds.set(id);

	if (!IsReservedEntityId(id))
	{
		if (id < s_lowestTakenEntityId)
		{
			s_lowestTakenEntityId = id;
		}
		if (id > s_highestTakenEntityId)
		{
			s_highestTakenEntityId = id;
		}
	}

	return ArgusEntity(id);
}

void ArgusEntity::DestroyEntity(ArgusEntity& entityToDestroy)
{
	if (!entityToDestroy)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to destroy an %s that doesn't exist."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return;
	}

	uint16 entityToDestoryId = entityToDestroy.GetId();
	s_takenEntityIds.set(entityToDestoryId, false);
	ArgusComponentRegistry::RemoveComponentsForEntity(entityToDestoryId);

	entityToDestroy = k_emptyEntity;
	
	if (IsReservedEntityId(entityToDestoryId))
	{
		return;
	}

	if (entityToDestoryId == s_lowestTakenEntityId)
	{
		uint16 newLowestTakenId = ArgusECSConstants::k_maxEntities;
		for (uint16 i = s_lowestTakenEntityId + 1u; i <= s_highestTakenEntityId; ++i)
		{
			if (s_takenEntityIds[i])
			{
				newLowestTakenId = i;
				break;
			}
		}

		s_lowestTakenEntityId = newLowestTakenId;
	}

	if (entityToDestoryId == s_highestTakenEntityId)
	{
		uint16 newHighestTakenId = 0u;
		for (uint16 i = s_highestTakenEntityId - 1u; i >= s_lowestTakenEntityId; --i)
		{
			if (s_takenEntityIds[i])
			{
				newHighestTakenId = i;
				break;
			}
		}

		s_highestTakenEntityId = newHighestTakenId;
	}
}

void ArgusEntity::DestroyEntity(uint16 entityIdToDestroy)
{
	ArgusEntity retrievedEntity = RetrieveEntity(entityIdToDestroy);
	DestroyEntity(retrievedEntity);
}

ArgusEntity ArgusEntity::RetrieveEntity(uint16 id)
{
	if (id < ArgusECSConstants::k_maxEntities && s_takenEntityIds[id])
	{
		return ArgusEntity(id);
	}

	return ArgusEntity::k_emptyEntity;
}

uint16 ArgusEntity::GetNextLowestUntakenId(uint16 lowestId)
{
	if (lowestId >= ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to retrieve an ID that is beyond %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities));
		return ArgusECSConstants::k_maxEntities;
	}

	while (lowestId < (ArgusECSConstants::k_maxEntities - 1) && s_takenEntityIds[lowestId])
	{
		lowestId++;
	}

	if (s_takenEntityIds[lowestId])
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Exceeded the maximum number of allowed %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return ArgusECSConstants::k_maxEntities;
	}

	return lowestId;
}

uint16 ArgusEntity::GetTeamEntityId(ETeam team)
{
	uint32 logValue = FMath::FloorLog2(static_cast<uint32>(team));
	return ArgusECSConstants::k_singletonEntityId - 1 - logValue;
}

ArgusEntity ArgusEntity::GetTeamEntity(ETeam team)
{
	return RetrieveEntity(GetTeamEntityId(team));
}

ArgusEntity::ArgusEntity(const ArgusEntity& other)
{
	m_id = other.GetId();
}

ArgusEntity& ArgusEntity::operator=(const ArgusEntity& other)
{
	m_id = other.GetId();
	return *this;
}

bool ArgusEntity::operator==(const ArgusEntity& other) const
{
	return m_id == other.GetId();
}

ArgusEntity::operator bool() const
{
	return DoesEntityExist(m_id);
}

ArgusEntity::ArgusEntity() : m_id(ArgusECSConstants::k_maxEntities)
{
}

ArgusEntity::ArgusEntity(uint16 id) : m_id(id)
{
}

uint16 ArgusEntity::GetId() const
{
	return m_id;
}

bool ArgusEntity::IsKillable() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	return GetComponent<HealthComponent>() != nullptr;
}

bool ArgusEntity::IsAlive() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	return taskComponent->m_baseState == BaseState::Alive;
}

bool ArgusEntity::IsMoveable() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const bool hasNavigationComponent = GetComponent<NavigationComponent>() != nullptr;
	const bool hasTransformComponent = GetComponent<TransformComponent>() != nullptr;
	const bool hasTargetingComponent = GetComponent<TargetingComponent>() != nullptr;

	return hasNavigationComponent && hasTransformComponent && hasTargetingComponent;
}

bool ArgusEntity::IsSelected() const
{
	const ArgusEntity singletonEntity = ArgusECSConstants::k_singletonEntityId;
	if (!singletonEntity)
	{
		return false;
	}

	const InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return false;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		if (inputInterfaceComponent->m_selectedArgusEntityIds[i] == m_id)
		{
			return true;
		}
	}

	return false;
}

bool ArgusEntity::IsIdle() const
{
	const TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	if (!IsAlive())
	{
		return false;
	}

	return	taskComponent->m_movementState == MovementState::None &&
			taskComponent->m_combatState == CombatState::None &&
			taskComponent->m_constructionState == ConstructionState::None &&
			taskComponent->m_spawningState == SpawningState::None;
}

#if !UE_BUILD_SHIPPING
const FString ArgusEntity::GetDebugString() const
{
	return FString::Printf(TEXT("(%s: %d)"), ARGUS_NAMEOF(m_id), m_id);
}
#endif //!UE_BUILD_SHIPPING
