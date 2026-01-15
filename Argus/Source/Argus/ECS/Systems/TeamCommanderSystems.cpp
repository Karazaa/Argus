// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "Systems/ResourceSystems.h"

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

void TeamCommanderSystems::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->m_idleEntityIdsForTeam.Reset();
	teamCommanderComponent->m_seenResourceSourceEntityIds.Reset();
	teamCommanderComponent->m_numResourceExtractors = 0u;
	teamCommanderComponent->m_numLivingUnits = 0u;
	teamCommanderComponent->m_priorities.Reset();
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

	teamCommanderComponent->m_numLivingUnits++;
	if (components.m_entity.IsIdle())
	{
		teamCommanderComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	if (components.m_taskComponent->m_resourceExtractionState != EResourceExtractionState::None)
	{
		teamCommanderComponent->m_numResourceExtractors++;
	}

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
			teamCommanderComponent->m_seenResourceSourceEntityIds.Add(components.m_entity.GetId());
		}
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

	const int32 areaIndex = GetRevealedAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent);
	teamCommanderComponent->m_revealedAreas[areaIndex] = true;
}

void TeamCommanderSystems::UpdateTeamCommanderPriorities(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPriorities);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
		priority.m_directive = directiveToEvaluate;
		priority.m_weight = 1.0f;
	}

	teamCommanderComponent->m_priorities.Sort();
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
		if (AssignIdleEntityToDirectiveIfAble(idleEntity, teamCommanderComponent, teamCommanderComponent->m_priorities[i].m_directive))
		{
			return;
		}
	}
}

bool TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, ETeamCommanderDirective directive)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToDirectiveIfAble);
	switch (directive)
	{
		case ETeamCommanderDirective::ExtractResources:
			return AssignEntityToResourceExtractionIfAble(idleEntity, teamCommanderComponent);
		case ETeamCommanderDirective::Scout:
			return AssignEntityToScoutingIfAble(idleEntity, teamCommanderComponent);
		default:
			break;
	}

	return false;
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
	for (int32 i = 0; i < teamCommanderComponent->m_seenResourceSourceEntityIds.Num(); ++i)
	{
		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(teamCommanderComponent->m_seenResourceSourceEntityIds[i]);
		if (!ResourceSystems::CanEntityExtractResourcesFromOtherEntity(entity, resourceSourceEntity))
		{
			continue;
		}

		const float distanceSquared = entity.GetDistanceSquaredToOtherEntity(resourceSourceEntity);
		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestEntity = resourceSourceEntity;
		}
	}

	if (!closestEntity)
	{
		return false;
	}

	targetingComponent->SetEntityTarget(closestEntity.GetId());
	taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
	taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;

	return true;
}

bool TeamCommanderSystems::AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToScoutingIfAble);
	ARGUS_RETURN_ON_NULL_BOOL(teamCommanderComponent, ArgusECSLog);

	return true;
}

int32 TeamCommanderSystems::GetRevealedAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
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
