// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "ArgusStaticData.h"
#include "DataComponentDefinitions/CarrierComponentData.h"
#include "DataComponentDefinitions/ResourceComponentData.h"
#include "DataComponentDefinitions/ResourceExtractionComponentData.h"
#include "Systems/AbilitySystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TransformSystems.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif // !UE_BUILD_SHIPPING

void TeamCommanderSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TeamCommanderSystems::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems::UpdateTeamCommanderPerEntity);
	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::UpdateTeamCommanderPriorities);
	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);
}

void TeamCommanderSystems::InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerWidth = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);
	const int32 numAreas = FMath::FloorToInt32(FMath::Square(areasPerWidth));
	teamCommanderComponent->m_revealedAreas.SetNum(numAreas, false);
}

void TeamCommanderSystems::PerformInitialUpdate()
{
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems::UpdateTeamCommanderPerEntity);
}

void TeamCommanderSystems::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->IterateAllSeenResourceSources([](ResourceSourceExtractionData& data)
	{
		ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceSinkEntityId), data);
		ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceExtractorEntityId), data);
		return false;
	});
	teamCommanderComponent->ResetUpdateArrays();
}

void TeamCommanderSystems::ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkEntity, ResourceSourceExtractionData& data)
{
	if (!existingResourceSinkEntity)
	{
		data.m_resourceSinkEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!existingResourceSinkEntity.IsAlive())
	{
		data.m_resourceSinkEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}
}

void TeamCommanderSystems::ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity existingResourceExtractorEntity, ResourceSourceExtractionData& data)
{
	if (!existingResourceExtractorEntity)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!existingResourceExtractorEntity.IsAlive())
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	TaskComponent* extractorTaskComponent = existingResourceExtractorEntity.GetComponent<TaskComponent>();
	TargetingComponent* extractorTargetingComponent = existingResourceExtractorEntity.GetComponent<TargetingComponent>();
	if (!extractorTaskComponent)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (extractorTaskComponent->m_directiveFromTeamCommander != ETeamCommanderDirective::ExtractResources)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!extractorTargetingComponent->HasEntityTarget())
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (extractorTargetingComponent->m_targetEntityId != data.m_resourceSourceEntityId && extractorTargetingComponent->m_targetEntityId != data.m_resourceSinkEntityId)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_identityComponent->m_team == ETeam::None)
	{
		ArgusEntity::IterateTeamEntities([&components](ArgusEntity teamCommanderEntity)
		{
			TeamCommanderSystems::UpdateTeamCommanderPerNeutralEntity(components, teamCommanderEntity);
		});
		return;
	}

	TeamCommanderComponent* teamCommanderComponent = ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	UpdateTeamCommanderPerEntityOnTeam(components, teamCommanderComponent);
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const bool isAlive = components.m_entity.IsAlive();
	if (isAlive && components.m_entity.IsIdle() && !components.m_entity.IsPassenger())
	{
		teamCommanderComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	UpdateResourceExtractionDataPerSink(components, teamCommanderComponent);
	UpdateRevealedAreasPerEntityOnTeam(components, teamCommanderComponent);
}

void TeamCommanderSystems::UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnOtherTeam);

	TeamCommanderComponent* teamCommanderComponent = teamCommanderEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (ResourceComponent* resourceComponent = components.m_entity.GetComponent<ResourceComponent>())
	{
		if (resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source && components.m_identityComponent->WasEverSeenBy(teamCommanderComponent->m_teamToCommand))
		{
			for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
			{
				EResourceType type = static_cast<EResourceType>(i);
				if (components.m_resourceComponent->m_currentResources.HasResourceType(type))
				{
					teamCommanderComponent->AddSeenResourceSourceIfNotPresent(type, components.m_entity.GetId());
				}
			}
		}
	}
}

void TeamCommanderSystems::UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_entity.IsAlive() || !components.m_resourceComponent || components.m_resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink)
	{
		return;
	}

	for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		EResourceType type = static_cast<EResourceType>(i);
		if (!components.m_resourceComponent->m_currentResources.HasResourceType(type))
		{
			continue;
		}

		teamCommanderComponent->IterateSeenResourceSourcesOfType(type, [&components](ResourceSourceExtractionData& data)
		{
			if (data.m_resourceSourceEntityId == ArgusECSConstants::k_maxEntities)
			{
				return false;
			}

			if (data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
			{
				return false;
			}

			ArgusEntity sourceEntity = ArgusEntity::RetrieveEntity(data.m_resourceSourceEntityId);
			if (TargetingSystems::IsInSightRangeOfOtherEntity(components.m_entity, sourceEntity))
			{
				data.m_resourceSinkEntityId = components.m_entity.GetId();
				return true;
			}

			return false;
		});
	}
}

void TeamCommanderSystems::UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateRevealedAreasPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return;
	}

	const int32 areaIndex = GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent);
	if (areaIndex >= 0)
	{
		teamCommanderComponent->m_revealedAreas[areaIndex] = true;
	}
}

void TeamCommanderSystems::UpdateTeamCommanderPriorities(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPriorities);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		switch (directiveToEvaluate)
		{
			case ETeamCommanderDirective::ConstructResourceSink:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_resourceType = static_cast<EResourceType>(j);
					UpdateConstructResourceSinkTeamPriority(teamCommanderComponent, priority);
				}
				continue;
			case ETeamCommanderDirective::ExtractResources:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_resourceType = static_cast<EResourceType>(j);
					UpdateResourceExtractionTeamPriority(teamCommanderComponent, priority);
				}
				continue;
			case ETeamCommanderDirective::SpawnUnit:
				for (uint8 j = 0u; j < static_cast<uint8>(ESpawnUnitType::Count); ++j)
				{
					ESpawnUnitType unitType = static_cast<ESpawnUnitType>(j);
					if (unitType == ESpawnUnitType::Extractor)
					{
						for (uint8 k = 0u; k < static_cast<uint8>(EResourceType::Count); ++k)
						{
							TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
							priority.m_directive = directiveToEvaluate;
							priority.m_unitType = unitType;
							priority.m_resourceType = static_cast<EResourceType>(k);
							UpdateSpawnUnitTeamPriority(teamCommanderComponent, priority);
						}
					}
					else
					{
						TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
						priority.m_directive = directiveToEvaluate;
						priority.m_unitType = unitType;
						UpdateSpawnUnitTeamPriority(teamCommanderComponent, priority);
					}
				}
				continue;
			case ETeamCommanderDirective::Scout:
			{
				TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
				priority.m_directive = directiveToEvaluate;
				UpdateScoutingTeamPriority(teamCommanderComponent, priority);
				continue;
			}
			default:
				continue;
		}
	}

	teamCommanderComponent->m_priorities.Sort();
}

void TeamCommanderSystems::UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateConstructResourceSinkTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ConstructResourceSink || priority.m_resourceType == EResourceType::Count)
	{
		return;
	}

	bool updated = teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceSinkEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight = 2.0f;
			return true;
		}

		return false;
	});

	if (updated)
	{
		return;
	}

	priority.m_weight = 0.0f;
}

void TeamCommanderSystems::UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateResourceExtractionTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ExtractResources)
	{
		return;
	}

	bool updated = teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceExtractorEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight = 1.0f;
			return true;
		}

		return false;
	});

	if (updated)
	{
		return;
	}

	priority.m_weight = 0.0f;
}

void TeamCommanderSystems::UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateSpawnUnitTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::SpawnUnit)
	{
		return;
	}

	if (priority.m_unitType == ESpawnUnitType::Carrier)
	{
		priority.m_weight = 0.5f;
		return;
	}

	float numUnassignedResourceSources = 0.0f;
	teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_resourceType, [&priority, &numUnassignedResourceSources](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceExtractorEntityId == ArgusECSConstants::k_maxEntities)
		{
			numUnassignedResourceSources += 1.0f;
		}

		return false;
	});

	priority.m_weight = numUnassignedResourceSources;
}

void TeamCommanderSystems::UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateScoutingTeamPriority);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::Scout)
	{
		return;
	}

	// TODO JAMES: We should probably massively de-prioritize this based on how much of the map has been discovered. 

	priority.m_weight = 1.0f;
}

void TeamCommanderSystems::ActUponUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	// TODO JAMES: Are there any circumstances in which we want to allow the AI to control a player team?
	if (teamCommanderComponent->m_teamToCommand == inputInterfaceComponent->m_activePlayerTeam)
	{
		return;
	}

	for (int32 i = 0; i < teamCommanderComponent->m_idleEntityIdsForTeam.Num(); ++i)
	{
		AssignIdleEntityToWork(ArgusEntity::RetrieveEntity(teamCommanderComponent->m_idleEntityIdsForTeam[i]), teamCommanderComponent);
	}
}

void TeamCommanderSystems::AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToWork);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (int32 i = 0; i < teamCommanderComponent->m_priorities.Num(); ++i)
	{
		if (AssignIdleEntityToDirectiveIfAble(idleEntity, teamCommanderComponent, teamCommanderComponent->m_priorities[i]))
		{
			return;
		}
	}
}

bool TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble);
	switch (priority.m_directive)
	{
		case ETeamCommanderDirective::ConstructResourceSink:
			return AssignEntityToConstructResourceSinkIfAble(idleEntity, teamCommanderComponent, priority);
		case ETeamCommanderDirective::ExtractResources:
			return AssignEntityToResourceExtractionIfAble(idleEntity, teamCommanderComponent);
		case ETeamCommanderDirective::SpawnUnit:
			return AssignEntityToSpawnUnitIfAble(idleEntity, teamCommanderComponent, priority);
		case ETeamCommanderDirective::Scout:
			return AssignEntityToScoutingIfAble(idleEntity, teamCommanderComponent);
		default:
			break;
	}

	return false;
}

bool TeamCommanderSystems::AssignEntityToConstructResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_resourceType == EResourceType::Count)
	{
		return false;
	}

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	TArray<TPair<const UAbilityRecord*, EAbilityIndex>> abilityIndexPairs;
	if (!GetConstructResourceSinkAbilities(entity, priority.m_resourceType, abilityIndexPairs))
	{
		return false;
	}

	if (!FindTargetLocForConstructResourceSink(entity, abilityIndexPairs, teamCommanderComponent, priority.m_resourceType))
	{
		return false;
	}

	taskComponent->m_abilityState = AbilitySystems::GetProcessAbilityStateForAbilityIndex(abilityIndexPairs[0].Value);
	taskComponent->m_directiveFromTeamCommander = priority.m_directive;
	priority.m_weight = 0.0f;
	teamCommanderComponent->m_priorities.Sort();

	return true;
}

bool TeamCommanderSystems::AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!taskComponent || !targetingComponent)
	{
		return false;
	}

	ArgusEntity closestEntity = ArgusEntity::k_emptyEntity;
	float closestDistanceSquared = FLT_MAX;
	ResourceSourceExtractionData* pointerToClosestExtractionData = nullptr;
	teamCommanderComponent->IterateAllSeenResourceSources([entity, &closestEntity, &closestDistanceSquared, &pointerToClosestExtractionData](ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId == ArgusECSConstants::k_maxEntities || data.m_resourceExtractorEntityId != ArgusECSConstants::k_maxEntities)
		{
			return false;
		}

		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(data.m_resourceSourceEntityId);
		if (!ResourceSystems::CanEntityExtractResourcesFromOtherEntity(entity, resourceSourceEntity))
		{
			return false;
		}

		const float distanceSquared = entity.GetDistanceSquaredToOtherEntity(resourceSourceEntity);
		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestEntity = resourceSourceEntity;
			pointerToClosestExtractionData = &data;
		}

		return false;
	});

	if (!closestEntity || !pointerToClosestExtractionData)
	{
		return false;
	}

	targetingComponent->SetEntityTarget(closestEntity.GetId());
	taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
	taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::ExtractResources;
	pointerToClosestExtractionData->m_resourceExtractorEntityId = entity.GetId();
	return true;
}

bool TeamCommanderSystems::AssignEntityToSpawnUnitIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToSpawnUnitIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	const SpawningComponent* spawningComponent = entity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		return false;
	}

	TArray<TPair<const UAbilityRecord*, EAbilityIndex>> abilityIndexPairs;
	if (!GetSpawnUnitAbilities(entity, priority.m_unitType, priority.m_resourceType, abilityIndexPairs))
	{
		return false;
	}

	return false;
}

bool TeamCommanderSystems::AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToScoutingIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	if (!entity.IsMoveable())
	{
		return false;
	}

	TeamCommanderSystemsArgs components;
	if (!components.PopulateArguments(entity) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return  false;
	}

	const int32 unrevealedIndex = GetClosestUnrevealedAreaToEntity(components, teamCommanderComponent);
	if (unrevealedIndex < 0)
	{
		return false;
	}

	components.m_targetingComponent->SetLocationTarget(GetWorldSpaceLocationFromAreaIndex(unrevealedIndex, teamCommanderComponent));
	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
	components.m_taskComponent->m_directiveFromTeamCommander = ETeamCommanderDirective::Scout;
	return true;
}

int32 TeamCommanderSystems::GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return -1;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float areasPerDimension = ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength);

	float xValue = ArgusMath::SafeDivide(components.m_transformComponent->m_location.Y + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);
	float yValue = ArgusMath::SafeDivide((-components.m_transformComponent->m_location.X) + spatialPartitioningComponent->m_validSpaceExtent, teamCommanderComponent->m_revealedAreaDimensionLength);

	int32 xValue32 = FMath::FloorToInt32(xValue);
	int32 yValue32 = FMath::FloorToInt32(yValue);

	return (yValue32 * FMath::FloorToInt32(areasPerDimension)) + xValue32;
}

FVector TeamCommanderSystems::GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, FVector::ZeroVector);
	if (areaIndex < 0)
	{
		return FVector::ZeroVector;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, FVector::ZeroVector);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));

	int32 xCoordinate, yCoordinate;
	ConvertAreaIndexToAreaCoordinates(areaIndex, areasPerDimension, xCoordinate, yCoordinate);

	const float xOffset = ((static_cast<float>(xCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);
	const float yOffset = ((static_cast<float>(yCoordinate) + 0.5f) * teamCommanderComponent->m_revealedAreaDimensionLength);

	FVector output = FVector::ZeroVector;
	output.Y = xOffset - spatialPartitioningComponent->m_validSpaceExtent;
	output.X = spatialPartitioningComponent->m_validSpaceExtent - yOffset;
	return output;
}

int32 TeamCommanderSystems::GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, -1);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, -1);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const int32 areasPerDimension = FMath::FloorToInt32(ArgusMath::SafeDivide(worldspaceWidth, teamCommanderComponent->m_revealedAreaDimensionLength));
	int32 entityXCoordinate, entityYCoordinate;
	ConvertAreaIndexToAreaCoordinates(GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent), areasPerDimension, entityXCoordinate, entityYCoordinate);

	if (entityXCoordinate < 0 || entityYCoordinate < 0)
	{
		return -1;
	}

	for (int32 i = 1; i < areasPerDimension; ++i)
	{
		const int32 leftBoundX = FMath::Max(entityXCoordinate - i, 0);
		const int32 rightBoundX = FMath::Min(entityXCoordinate + i, (areasPerDimension - 1));
		const int32 upperBoundY = FMath::Max(entityYCoordinate - i, 0);
		const int32 lowerBoundY = FMath::Min(entityYCoordinate + i, (areasPerDimension - 1));

		TArray<int32> validAreasNearby;
		for (int32 j = -i; j <= i; ++j)
		{
			const int32 xBoundY = FMath::Min(FMath::Max(entityYCoordinate + j, 0), (areasPerDimension - 1));
			const int32 yBoundX = FMath::Min(FMath::Max(entityXCoordinate + j, 0), (areasPerDimension - 1));

			int32 indexToCheck;
			ConvertAreaCoordinatesToAreaIndex(leftBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(rightBoundX, xBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, upperBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
			ConvertAreaCoordinatesToAreaIndex(yBoundX, lowerBoundY, areasPerDimension, indexToCheck);
			if (indexToCheck >= 0 && !teamCommanderComponent->m_revealedAreas[indexToCheck])
			{
				validAreasNearby.Add(indexToCheck);
			}
		}

		if (validAreasNearby.Num() > 0)
		{
			return validAreasNearby[FMath::RandRange(0, validAreasNearby.Num() - 1)];
		}
	}

	return -1;
}

void TeamCommanderSystems::ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate)
{
	if (areaIndex < 0)
	{
		xCoordinate = -1;
		yCoordinate = -1;
	}

	yCoordinate = ArgusMath::SafeDivide(areaIndex, areasPerDimension);
	xCoordinate = areaIndex % areasPerDimension;
}

void TeamCommanderSystems::ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex)
{
	areaIndex = (yCoordinate * areasPerDimension) + xCoordinate;
}

bool TeamCommanderSystems::GetConstructResourceSinkAbilities(ArgusEntity entity, EResourceType type, TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& outAbilityIndexPairs)
{
	ARGUS_TRACE(TeamCommanderSystems::GetConstructResourceSinkAbilities);

	outAbilityIndexPairs.Reset();
	outAbilityIndexPairs.Reserve(ArgusECSConstants::k_numEntityAbilities);
	AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return false;
	}

	abilityComponent->IterateActiveAbilityIds([entity, type, &outAbilityIndexPairs](uint32 abilityRecordId, EAbilityIndex iteratedAbilityIndex)
	{
		const UAbilityRecord* record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityRecordId);
		if (!record)
		{
			return;
		}

		if (ResourceSystems::CanEntityAffordTeamResourceChange(entity, record->m_requiredResourceChangeToCast) && DoesAbilityConstructResourceSink(record, type))
		{
			outAbilityIndexPairs.Add(TPair<const UAbilityRecord*, EAbilityIndex>(record, iteratedAbilityIndex));
		}
	});

	return outAbilityIndexPairs.Num() > 0;
}

bool TeamCommanderSystems::GetSpawnUnitAbilities(ArgusEntity entity, ESpawnUnitType unitType, EResourceType resourceType, TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& outAbilityIndexPairs)
{
	ARGUS_TRACE(TeamCommanderSystems::GetSpawnUnitAbilities);

	outAbilityIndexPairs.Reset();
	outAbilityIndexPairs.Reserve(ArgusECSConstants::k_numEntityAbilities);
	AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return false;
	}

	abilityComponent->IterateActiveAbilityIds([entity, unitType, resourceType, &outAbilityIndexPairs](uint32 abilityRecordId, EAbilityIndex iteratedAbilityIndex)
	{
		const UAbilityRecord* record = ArgusStaticData::GetRecord<UAbilityRecord>(abilityRecordId);
		if (!record)
		{
			return;
		}

		if (ResourceSystems::CanEntityAffordTeamResourceChange(entity, record->m_requiredResourceChangeToCast) && DoesAbilitySpawnUnitType(record, unitType, resourceType))
		{
			outAbilityIndexPairs.Add(TPair<const UAbilityRecord*, EAbilityIndex>(record, iteratedAbilityIndex));
		}
	});

	return outAbilityIndexPairs.Num() > 0;
}

bool TeamCommanderSystems::DoesAbilityConstructResourceSink(const UAbilityRecord* abilityRecord, EResourceType type)
{
	ARGUS_TRACE(TeamCommanderSystems::DoesAbilityConstructResourceSink);

	ARGUS_RETURN_ON_NULL_BOOL(abilityRecord, ArgusECSLog);

	const UArgusEntityTemplate* entityTemplate = AbilitySystems::GetEntityTemplateForAbility(abilityRecord, EAbilityTypes::Construct);
	if (!entityTemplate)
	{
		return false;
	}

	const UResourceComponentData* resourceComponentData = entityTemplate->GetComponentFromTemplate<UResourceComponentData>();
	if (!resourceComponentData || resourceComponentData->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink)
	{
		return false;
	}

	return resourceComponentData->m_currentResources.HasResourceType(type);
}

bool TeamCommanderSystems::DoesAbilitySpawnUnitType(const UAbilityRecord* abilityRecord, ESpawnUnitType unitType, EResourceType resourceType)
{
	ARGUS_TRACE(TeamCommanderSystems::DoesAbilitySpawnUnitType);

	ARGUS_RETURN_ON_NULL_BOOL(abilityRecord, ArgusECSLog);

	const UArgusEntityTemplate* entityTemplate = AbilitySystems::GetEntityTemplateForAbility(abilityRecord, EAbilityTypes::Spawn);
	if (!entityTemplate)
	{
		return false;
	}

	switch (unitType)
	{
		case ESpawnUnitType::Carrier:
			if (const UCarrierComponentData* carrierComponentData = entityTemplate->GetComponentFromTemplate<UCarrierComponentData>())
			{
				return true;
			}
			return false;
		case ESpawnUnitType::Extractor:
			if (const UResourceExtractionComponentData* resourceExtractionComponentData = entityTemplate->GetComponentFromTemplate<UResourceExtractionComponentData>())
			{
				if (const UResourceSetRecord* resourceSetRecord = resourceExtractionComponentData->m_resourcesToExtractRecordId.LoadSynchronous())
				{
					return resourceSetRecord->m_resourceSet.HasResourceType(resourceType);
				}
			}
			return false;
		default:
			return false;
	}
}

bool TeamCommanderSystems::FindTargetLocForConstructResourceSink(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type)
{
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(worldReferenceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_BOOL(worldReferenceComponent->m_worldPointer, ArgusECSLog);

	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return false;
	}

	int32 index = -1;
	ArgusEntity nearestResourceSource = GetNearestSeenResourceSourceToEntity(entity, abilityIndexPairs, teamCommanderComponent, type, index);
	if (!nearestResourceSource || index < 0)
	{
		return false;
	}

	TransformComponent* resourceSourceTransformComponent = nearestResourceSource.GetComponent<TransformComponent>();
	TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(resourceSourceTransformComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_BOOL(transformComponent, ArgusECSLog);

	FVector2D fromSinkToEntity = FVector2D(transformComponent->m_location - resourceSourceTransformComponent->m_location).GetSafeNormal();

	const float safeZoneDistance = AbilitySystems::GetResourceBufferRadiusOfConstructionAbility(abilityIndexPairs[index].Key);
	const float radiusDistance = AbilitySystems::GetRaidusOfConstructionAbility(abilityIndexPairs[index].Key);

	FVector2D candidateOffset = (fromSinkToEntity * (safeZoneDistance + resourceSourceTransformComponent->m_radius + ArgusECSConstants::k_resourceSinkBufferDistanceAdjustment));
	FVector candidatePoint = FVector(candidateOffset, 0.0f) + resourceSourceTransformComponent->m_location;
	bool isBlocked = SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(candidatePoint, radiusDistance, safeZoneDistance);
	if (isBlocked)
	{
		const int32 numIterations = 16;
		const float angleOffset = ArgusMath::SafeDivide(360.0f, static_cast<float>(numIterations));

		DrawDebugSphere(worldReferenceComponent->m_worldPointer, candidatePoint, 5.0f, 10, FColor::Red, false, 10.0f);

		for (int32 i = 0; i < numIterations; ++i)
		{
			candidateOffset = candidateOffset.GetRotated(angleOffset);
			candidatePoint.X = resourceSourceTransformComponent->m_location.X + candidateOffset.X;
			candidatePoint.Y = resourceSourceTransformComponent->m_location.Y + candidateOffset.Y;

			if (!SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(candidatePoint, radiusDistance, safeZoneDistance))
			{
				DrawDebugSphere(worldReferenceComponent->m_worldPointer, candidatePoint, 5.0f, 10, FColor::Green, false, 10.0f);
				isBlocked = false;
				break;
			}
			else
			{
				DrawDebugSphere(worldReferenceComponent->m_worldPointer, candidatePoint, 5.0f, 10, FColor::Red, false, 10.0f);
			}
		}
	}

	if (isBlocked)
	{
		return false;
	}

	targetingComponent->SetLocationTarget(TransformSystems::ProjectLocationOntoNavigationData(worldReferenceComponent->m_worldPointer, radiusDistance, candidatePoint));
	return true;
}

ArgusEntity TeamCommanderSystems::GetNearestSeenResourceSourceToEntity(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type, int32& outPairIndex)
{
	ARGUS_RETURN_ON_NULL_VALUE(teamCommanderComponent, ArgusECSLog, ArgusEntity::k_emptyEntity);

	outPairIndex = 0;

	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return ArgusEntity::k_emptyEntity;
	}

	ArgusEntity nearestResourceSource = ArgusEntity::k_emptyEntity;
	float minDistanceSquared = FLT_MAX;

	teamCommanderComponent->IterateSeenResourceSourcesOfType(type, [transformComponent, &abilityIndexPairs, &nearestResourceSource, &minDistanceSquared, &outPairIndex](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
		{
			return false;
		}

		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(data.m_resourceSourceEntityId);
		TransformComponent* resourceSinkSourceTransformComponent = resourceSourceEntity.GetComponent<TransformComponent>();
		if (!resourceSinkSourceTransformComponent)
		{
			return false;
		}

		bool anyValidAbilities = false;
		int32 j;
		for (j = 0; j < abilityIndexPairs.Num(); ++j)
		{
			if (ResourceSystems::CanEntityTemplateActAsSinkToEntitySource(AbilitySystems::GetEntityTemplateForAbility(abilityIndexPairs[j].Key, EAbilityTypes::Construct), resourceSourceEntity))
			{
				anyValidAbilities = true;
				break;
			}
		}

		if (!anyValidAbilities)
		{
			return false;
		}

		const float distanceSquared = FVector::DistSquared2D(transformComponent->m_location, resourceSinkSourceTransformComponent->m_location);
		if (distanceSquared < minDistanceSquared)
		{
			minDistanceSquared = distanceSquared;
			nearestResourceSource = resourceSourceEntity;
			outPairIndex = j;
		}

		return false;
	});

	return nearestResourceSource;
}

#if !UE_BUILD_SHIPPING
void TeamCommanderSystems::DebugRevealedAreasForTeamEntityId(uint16 teamEntityId)
{
	const TeamCommanderComponent* teamCommanderComponent = ArgusEntity::RetrieveEntity(teamEntityId).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	const WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>();
	ARGUS_RETURN_ON_NULL(worldReferenceComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(worldReferenceComponent->m_worldPointer, ArgusECSLog);

	const float halfDimension = ArgusMath::SafeDivide(teamCommanderComponent->m_revealedAreaDimensionLength, 2.0f);
	const FVector bounds = FVector(halfDimension, halfDimension, 0.0f);
	const FVector heightAdjust = FVector(0.0f, 0.0f, 5.0f);
	teamCommanderComponent->IterateRevealedAreas(true, [worldReferenceComponent, teamCommanderComponent, &bounds, &heightAdjust](int32 areaIndex)
	{
		DrawDebugBox(worldReferenceComponent->m_worldPointer, GetWorldSpaceLocationFromAreaIndex(areaIndex, teamCommanderComponent) + heightAdjust, bounds, FColor::Green);
	});
}
#endif // !UE_BUILD_SHIPPING
