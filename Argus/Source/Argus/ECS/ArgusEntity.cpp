// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMemorySource.h"
#include "ArgusStaticData.h"

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

	if (UNLIKELY(id == ArgusECSConstants::k_maxEntities))
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an invalid ID value."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return k_emptyEntity;
	}

	if (UNLIKELY(s_takenEntityIds[id]))
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
	if (UNLIKELY(!entityToDestroy))
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
	if (UNLIKELY(lowestId >= ArgusECSConstants::k_maxEntities))
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to retrieve an ID that is beyond %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities));
		return ArgusECSConstants::k_maxEntities;
	}

	while (lowestId < (ArgusECSConstants::k_maxEntities - 1) && s_takenEntityIds[lowestId])
	{
		lowestId++;
	}

	if (UNLIKELY(s_takenEntityIds[lowestId]))
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

	return taskComponent->m_baseState == EBaseState::Alive;
}

bool ArgusEntity::IsMoveable() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	if (!IsAlive() || IsPassenger())
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
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

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
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	if (!IsAlive())
	{
		return false;
	}

	return	taskComponent->m_movementState == EMovementState::None &&
			taskComponent->m_abilityState == EAbilityState::None &&
			taskComponent->m_combatState == ECombatState::None &&
			taskComponent->m_constructionState == EConstructionState::None &&
			taskComponent->m_spawningState == ESpawningState::None &&
			taskComponent->m_resourceExtractionState == EResourceExtractionState::None;
}

bool ArgusEntity::IsInRangeOfOtherEntity(const ArgusEntity& other, float range) const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const TransformComponent* transformComponent = GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return false;
	}

	const TransformComponent* otherTransformComponent = other.GetComponent<TransformComponent>();
	if (!otherTransformComponent)
	{
		return false;
	}

	float combinedRadius = transformComponent->m_radius + otherTransformComponent->m_radius;
	return (FVector::DistSquared(transformComponent->m_location, otherTransformComponent->m_location) - FMath::Square(combinedRadius)) <= FMath::Square(range);
}

bool ArgusEntity::IsPassenger() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const PassengerComponent* passengerComponent = GetComponent<PassengerComponent>();
	if (!passengerComponent)
	{
		return false;
	}

	return passengerComponent->m_carrierEntityId != ArgusECSConstants::k_maxEntities;
}

bool ArgusEntity::IsCarryingPassengers() const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const CarrierComponent* carrierComponent = GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		return false;
	}

	return carrierComponent->m_passengerEntityIds.Num() > 0;
}

bool ArgusEntity::IsOnTeam(ETeam team) const
{
	if (!DoesEntityExist(m_id))
	{
		return false;
	}

	const IdentityComponent* identityComponent = GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return false;
	}

	return identityComponent->m_team == team;
}

bool ArgusEntity::IsOnSameTeamAsOtherEntity(const ArgusEntity& otherEntity) const
{
	if (!otherEntity)
	{
		return false;
	}

	const IdentityComponent* otherIdentityComponent = otherEntity.GetComponent<IdentityComponent>();
	if (!otherIdentityComponent)
	{
		return false;
	}

	return IsOnTeam(otherIdentityComponent->m_team);
}


FVector ArgusEntity::GetCurrentTargetLocation() const
{
	const TargetingComponent* targetingComponent = GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		// TODO JAMES: Error here
		return FVector::ZeroVector;
	}

	if (!targetingComponent->HasAnyTarget())
	{
		// TODO JAMES: Error here
		return FVector::ZeroVector;
	}

	if (targetingComponent->HasLocationTarget())
	{
		return targetingComponent->m_targetLocation.GetValue();
	}

	TransformComponent* targetEntityTransformComponent = RetrieveEntity(targetingComponent->m_targetEntityId).GetComponent<TransformComponent>();
	if (!targetEntityTransformComponent)
	{
		// TODO JAMES: Error here
		return FVector::ZeroVector;
	}

	return targetEntityTransformComponent->m_location;
}

const UArgusActorRecord* ArgusEntity::GetAssociatedActorRecord() const
{
	if (!DoesEntityExist(m_id))
	{
		return nullptr;
	}

	const TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return nullptr;
	}

	const uint32 argusActorRecordId = taskComponent->m_spawnedFromArgusActorRecordId;
	if (argusActorRecordId == 0u)
	{
		return nullptr;
	}

	return ArgusStaticData::GetRecord<UArgusActorRecord>(argusActorRecordId);
}

#if !UE_BUILD_SHIPPING
const FString ArgusEntity::GetDebugString() const
{
	return FString::Printf(TEXT("(%s: %d)"), ARGUS_NAMEOF(m_id), m_id);
}
#endif //!UE_BUILD_SHIPPING
