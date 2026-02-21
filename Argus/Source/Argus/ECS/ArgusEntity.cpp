// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMemorySource.h"
#include "ArgusStaticData.h"
#include "Systems/FlockingSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

const ArgusEntity ArgusEntity::k_emptyEntity = ArgusEntity();
TBitArray<ArgusContainerAllocator<ArgusECSConstants::k_numBitBuckets> > ArgusEntity::s_takenEntityIds;

uint16 ArgusEntity::s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
uint16 ArgusEntity::s_highestTakenEntityId = 0u;

void ArgusEntity::FlushAllEntities()
{
	ArgusComponentRegistry::FlushAllComponents();
	s_takenEntityIds.Reset();
	s_takenEntityIds.SetNum(ArgusECSConstants::k_maxEntities, false);
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

uint16 ArgusEntity::GetHighestNonReservedEntityId()
{
	uint16 sizeOfTeamEnum = sizeof(ETeam) * 8;
	return (ArgusECSConstants::k_singletonEntityId - (sizeOfTeamEnum + 1));
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

	s_takenEntityIds[id] = true;

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
	s_takenEntityIds[entityToDestoryId] = false;
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

uint16 ArgusEntity::GetTeamOffset(ETeam team)
{
	return static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)));
}

uint16 ArgusEntity::GetTeamEntityId(ETeam team)
{
	return ArgusECSConstants::k_singletonEntityId - 1u - GetTeamOffset(team);
}

ArgusEntity ArgusEntity::GetTeamEntity(ETeam team)
{
	return RetrieveEntity(GetTeamEntityId(team));
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

bool ArgusEntity::IsKillable() const
{
	return GetComponent<HealthComponent>() != nullptr;
}

bool ArgusEntity::IsAlive() const
{
	TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	return taskComponent->m_baseState == EBaseState::Alive;
}

bool ArgusEntity::IsMoveable() const
{
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

	return	taskComponent->m_movementState == EMovementState::None &&
			taskComponent->m_abilityState == EAbilityState::None &&
			taskComponent->m_combatState == ECombatState::None &&
			taskComponent->m_constructionState == EConstructionState::None &&
			taskComponent->m_spawningState == ESpawningState::None &&
			taskComponent->m_resourceExtractionState == EResourceExtractionState::None;
}

bool ArgusEntity::IsInRangeOfOtherEntity(ArgusEntity otherEntity, float range) const
{
	const TransformComponent* transformComponent = GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return false;
	}

	const TransformComponent* otherTransformComponent = otherEntity.GetComponent<TransformComponent>();
	if (!otherTransformComponent)
	{
		return false;
	}

	float combinedRadius = transformComponent->m_radius + otherTransformComponent->m_radius;
	return (FVector::Dist2D(transformComponent->m_location, otherTransformComponent->m_location) - combinedRadius) <= range;
}

bool ArgusEntity::IsPassenger() const
{
	const PassengerComponent* passengerComponent = GetComponent<PassengerComponent>();
	if (!passengerComponent)
	{
		return false;
	}

	return passengerComponent->m_carrierEntityId != ArgusECSConstants::k_maxEntities;
}

bool ArgusEntity::IsCarryingPassengers() const
{
	const CarrierComponent* carrierComponent = GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		return false;
	}

	return carrierComponent->m_passengerEntityIds.Num() > 0;
}

bool ArgusEntity::IsOnTeam(ETeam team) const
{
	const IdentityComponent* identityComponent = GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return false;
	}

	return identityComponent->m_team == team;
}

bool ArgusEntity::IsOnSameTeamAsOtherEntity(ArgusEntity otherEntity) const
{
	const IdentityComponent* otherIdentityComponent = otherEntity.GetComponent<IdentityComponent>();
	if (!otherIdentityComponent)
	{
		return false;
	}

	return IsOnTeam(otherIdentityComponent->m_team);
}

bool ArgusEntity::IsOnPlayerTeam() const
{
	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);

	return IsOnTeam(inputInterfaceComponent->m_activePlayerTeam);
}

bool ArgusEntity::IsFlying() const
{
	const TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	return taskComponent->m_flightState == EFlightState::Flying;
}

bool ArgusEntity::IsUnderConstruction() const
{
	const TaskComponent* taskComponent = GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	return taskComponent->m_constructionState == EConstructionState::BeingConstructed;
}

bool ArgusEntity::CanFly() const
{
	const TransformComponent* transformComponent = GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return false;
	}

	return transformComponent->m_flightCapability != EFlightCapability::OnlyGrounded;
}

bool ArgusEntity::DoesEntitySatisfyEntityCategory(EntityCategory entityCategory) const
{
	switch (entityCategory.m_entityCategoryType)
	{
		case EEntityCategoryType::Carrier:
			if (const CarrierComponent* carrierComponent = GetComponent<CarrierComponent>())
			{
				return true;
			}
			return false;
		case EEntityCategoryType::Extractor:
			if (const ResourceExtractionComponent* resourceExtractionComponent = GetComponent<ResourceExtractionComponent>())
			{
				if (const UResourceSetRecord* resourceSetRecord = ArgusStaticData::GetRecord<UResourceSetRecord>(resourceExtractionComponent->m_resourcesToExtractRecordId))
				{
					return resourceSetRecord->m_resourceSet.HasResourceType(entityCategory.m_resourceType);
				}
			}
			return false;
		case EEntityCategoryType::ResourceSink:
			if (const ResourceComponent* resourceComponent = GetComponent<ResourceComponent>())
			{
				return resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Sink && resourceComponent->m_currentResources.HasResourceType(entityCategory.m_resourceType);
			}
			return false;
		default:
			return false;
	}
}

FVector ArgusEntity::GetCurrentTargetLocation() const
{
	const TargetingComponent* targetingComponent = GetComponent<TargetingComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(targetingComponent, ArgusECSLog, FVector::ZeroVector);

	if (!targetingComponent->HasAnyTarget())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Trying to get target location, but the entity has no target!"), ARGUS_FUNCNAME);
		return FVector::ZeroVector;
	}

	if (targetingComponent->HasLocationTarget())
	{
		return targetingComponent->m_targetLocation.GetValue();
	}

	TransformComponent* targetEntityTransformComponent = RetrieveEntity(targetingComponent->m_targetEntityId).GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(targetEntityTransformComponent, ArgusECSLog, FVector::ZeroVector);

	return targetEntityTransformComponent->m_location;
}

float ArgusEntity::GetDistanceSquaredToOtherEntity(ArgusEntity otherEntity) const
{
	const TransformComponent* transformComponent = GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(transformComponent, ArgusECSLog, 0.0f);

	const TransformComponent* otherTransformComponent = otherEntity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(otherTransformComponent, ArgusECSLog, 0.0f);

	return FVector::DistSquared(transformComponent->m_location, otherTransformComponent->m_location);
}

float ArgusEntity::GetDistanceToOtherEntity(ArgusEntity other) const
{
	return FMath::Sqrt(GetDistanceSquaredToOtherEntity(other));
}

const UArgusActorRecord* ArgusEntity::GetAssociatedActorRecord() const
{
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

void ArgusEntity::Destroy()
{
	ArgusEntity::DestroyEntity(*this);
}

#if !UE_BUILD_SHIPPING
const FString ArgusEntity::GetDebugString() const
{
	if (ArgusECSDebugger::ShouldShowFlockingDebugForEntity(m_id))
	{
		ArgusEntity flockingRoot = FlockingSystems::GetFlockingRootEntity(*this);
		return FString::Printf(TEXT("(%s: %d) \n (%s: %d)"), ARGUS_NAMEOF(m_id), m_id, ARGUS_NAMEOF(flockingRoot), flockingRoot.GetId());
	}
	else
	{
		return FString::Printf(TEXT("(%s: %d)"), ARGUS_NAMEOF(m_id), m_id);
	}
}
#endif //!UE_BUILD_SHIPPING
