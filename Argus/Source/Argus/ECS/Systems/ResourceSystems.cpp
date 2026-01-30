// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSystems.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "DataComponentDefinitions/ResourceComponentData.h"
#include "Systems/TargetingSystems.h"

void ResourceSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(ResourceSystems::RunSystems);

	ArgusEntity::IterateSystemsArgs<ResourceSystemsArgs>([deltaTime](ResourceSystemsArgs& components) 
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		ProcessResourceExtractionState(components);
	});
};

void ResourceSystems::ProcessResourceExtractionState(const ResourceSystemsArgs& components)
{
	ARGUS_TRACE(ResourceSystems::ProcessResourceExtractionState);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_resourceExtractionState)
	{
		case EResourceExtractionState::None:
			break;
		case EResourceExtractionState::DispatchedToExtract:
		case EResourceExtractionState::Extracting:
			ProcessResourceExtractionTiming(components);
			break;
		case EResourceExtractionState::DispatchedToDeposit:
		case EResourceExtractionState::Depositing:
			ProcessResourceDepositing(components);
			break;
	}
}

void ResourceSystems::ProcessResourceExtractionTiming(const ResourceSystemsArgs& components)
{
	ARGUS_TRACE(ResourceSystems::ProcessResourceExtractionTiming);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempted to extract resources for entity %d without having an entity target!"), ARGUS_FUNCNAME, components.m_entity.GetId());
		ClearResourceGatheringForEntity(components);
		return;
	}

	const ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!TargetingSystems::IsInMeleeRangeOfOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
		return;
	}
	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::Extracting;

	if (!CanEntityExtractResourcesFromOtherEntity(components.m_entity, targetEntity))
	{
		ClearResourceGatheringForEntity(components);
		return;
	}

	if (components.m_resourceExtractionComponent->m_resourceExtractionTimer.IsTimerTicking(components.m_entity))
	{
		return;
	}

	if (components.m_resourceExtractionComponent->m_lastExtractionSourceEntityId == ArgusECSConstants::k_maxEntities)
	{
		components.m_resourceExtractionComponent->m_lastExtractionSourceEntityId = components.m_targetingComponent->m_targetEntityId;
	}

	if (components.m_resourceExtractionComponent->m_resourceExtractionTimer.IsTimerComplete(components.m_entity))
	{
		components.m_resourceExtractionComponent->m_resourceExtractionTimer.FinishTimerHandling();
		if (ExtractResources(components))
		{
			components.m_resourceExtractionComponent->m_resourceExtractionTimer.StartTimer(components.m_resourceExtractionComponent->m_extractionLengthSeconds);
		}
		else
		{
			MoveToNearestDepositSink(components);
		}
	}
	else
	{
		components.m_resourceExtractionComponent->m_resourceExtractionTimer.StartTimer(components.m_resourceExtractionComponent->m_extractionLengthSeconds);
	}
}

void ResourceSystems::ProcessResourceDepositing(const ResourceSystemsArgs& components)
{
	ARGUS_TRACE(ResourceSystems::ProcessResourceDepositing);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::None;
		return;
	}

	const ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!TargetingSystems::IsInMeleeRangeOfOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToDeposit;
		return;
	}
	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::Depositing;

	if (!CanEntityDepositResourcesToOtherEntity(components.m_entity, targetEntity))
	{
		return;
	}

	DepositResources(components, targetEntity);
}

bool ResourceSystems::ExtractResources(const ResourceSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!targetEntity)
	{
		return false;
	}

	ResourceComponent* extractionTargetResourceComponent = targetEntity.GetComponent<ResourceComponent>();
	const UResourceSetRecord* extractionResourceRecord = ArgusStaticData::GetRecord<UResourceSetRecord>(components.m_resourceExtractionComponent->m_resourcesToExtractRecordId);
	if (!extractionResourceRecord || !extractionTargetResourceComponent)
	{
		return false;
	}

	const UResourceSetRecord* resourceCapacityRecord = ArgusStaticData::GetRecord<UResourceSetRecord>(components.m_resourceComponent->m_resourceCapacityRecordId);
	TransferResourcesBetweenComponents(extractionTargetResourceComponent, components.m_resourceComponent, extractionResourceRecord->m_resourceSet, resourceCapacityRecord);
	if (extractionTargetResourceComponent->m_currentResources.IsEmpty())
	{
		if (TaskComponent* targetTaskComponent = targetEntity.GetComponent<TaskComponent>())
		{
			targetTaskComponent->SetToKillState();
		}
		return false;
	}

	if (resourceCapacityRecord)
	{
		return !components.m_resourceComponent->m_currentResources.IsEntirelyAtCap(resourceCapacityRecord->m_resourceSet.MaskResourceSet(extractionTargetResourceComponent->m_currentResources));
	}

	return true;
}

void ResourceSystems::DepositResources(const ResourceSystemsArgs& components, ArgusEntity targetEntity)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const ResourceComponent* resourceComponent = targetEntity.GetComponent<ResourceComponent>();
	if (!resourceComponent || resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink)
	{
		ClearResourceGatheringForEntity(components);
		return;
	}

	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForEntity(components.m_entity);
	ARGUS_RETURN_ON_NULL(teamResourceComponent, ArgusECSLog);

	TransferResourcesBetweenComponents(components.m_resourceComponent, teamResourceComponent, components.m_resourceComponent->m_currentResources.MaskResourceSet(resourceComponent->m_currentResources));
	MoveToLastExtractionSource(components);
}

void ResourceSystems::MoveToNearestDepositSink(const ResourceSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter = [&components](const ArgusEntityKDTreeNode* entityNode)
	{
		ARGUS_RETURN_ON_NULL_BOOL(entityNode, ArgusECSLog);
		if (entityNode->m_entityId == components.m_entity.GetId())
		{
			return false;
		}

		return CanEntityDepositResourcesToOtherEntity(components.m_entity, ArgusEntity::RetrieveEntity(entityNode->m_entityId));
	};

	const uint16 targetDepositEntityId = spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdClosestToArgusEntity(components.m_entity, queryFilter);
	if (targetDepositEntityId == ArgusECSConstants::k_maxEntities)
	{
		ClearResourceGatheringForEntity(components);
		return;
	}

	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::Depositing;
	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	components.m_targetingComponent->m_targetEntityId = targetDepositEntityId;
}

void ResourceSystems::MoveToLastExtractionSource(const ResourceSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_resourceExtractionComponent->m_lastExtractionSourceEntityId == ArgusECSConstants::k_maxEntities)
	{
		ClearResourceGatheringForEntity(components);
		return;
	}

	ArgusEntity lastExtractionSource = ArgusEntity::RetrieveEntity(components.m_resourceExtractionComponent->m_lastExtractionSourceEntityId);
	if (!lastExtractionSource || !lastExtractionSource.IsAlive())
	{
		ClearResourceGatheringForEntity(components);
		return;
	}

	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::DispatchedToExtract;
	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	components.m_targetingComponent->m_targetEntityId = components.m_resourceExtractionComponent->m_lastExtractionSourceEntityId;
}

bool ResourceSystems::CanEntityActAsSinkToAnotherEntitySource(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity || !entity.IsAlive() || !otherEntity.IsAlive())
	{
		return false;
	}

	const ResourceComponent* resourceComponent = entity.GetComponent<ResourceComponent>();
	const ResourceComponent* otherResourceComponent = otherEntity.GetComponent<ResourceComponent>();
	if (!resourceComponent || !otherResourceComponent || resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink || otherResourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Source)
	{
		return false;
	}

	return !otherResourceComponent->m_currentResources.MaskResourceSet(resourceComponent->m_currentResources).IsEmpty();
}

bool ResourceSystems::CanEntityTemplateActAsSinkToEntitySource(const UArgusEntityTemplate* entityTemplate, ArgusEntity otherEntity)
{
	if (!entityTemplate || !otherEntity)
	{
		return false;
	}

	const UResourceComponentData* resourceComponentData = entityTemplate->GetComponentFromTemplate<UResourceComponentData>();
	const ResourceComponent* otherResourceComponent = otherEntity.GetComponent<ResourceComponent>();
	if (!resourceComponentData || !otherResourceComponent || resourceComponentData->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink || otherResourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Source)
	{
		return false;
	}

	return !otherResourceComponent->m_currentResources.MaskResourceSet(resourceComponentData->m_currentResources).IsEmpty();
}

bool ResourceSystems::CanEntityExtractResourcesFromOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	if (!otherEntity.IsAlive())
	{
		return false;
	}

	const ResourceComponent* resourceComponent = entity.GetComponent<ResourceComponent>();
	if (!resourceComponent || resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Carrier)
	{
		return false;
	}

	const ResourceExtractionComponent* resourceExtractionComponent = entity.GetComponent<ResourceExtractionComponent>();
	if (!resourceExtractionComponent)
	{
		return false;
	}

	const ResourceComponent* otherEntityResourceComponent = otherEntity.GetComponent<ResourceComponent>();
	if (!otherEntityResourceComponent || otherEntityResourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Source)
	{
		return false;
	}

	const UResourceSetRecord* extractionResourceRecord = ArgusStaticData::GetRecord<UResourceSetRecord>(resourceExtractionComponent->m_resourcesToExtractRecordId);
	if (!extractionResourceRecord)
	{
		return false;
	}

	return otherEntityResourceComponent->m_currentResources.CanAffordResourceChange(extractionResourceRecord->m_resourceSet);
}

bool ResourceSystems::CanEntityDepositResourcesToOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	if (!entity.IsAlive() || !otherEntity.IsAlive())
	{
		return false;
	}

	if (!entity.IsOnSameTeamAsOtherEntity(otherEntity))
	{
		return false;
	}

	const ResourceComponent* resourceComponent = entity.GetComponent<ResourceComponent>();
	if (!resourceComponent || resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Carrier)
	{
		return false;
	}

	const ResourceComponent* otherEntityResourceComponent = otherEntity.GetComponent<ResourceComponent>();
	if (!otherEntityResourceComponent || otherEntityResourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink)
	{
		return false;
	}

	if (const TaskComponent* otherEntityTaskComponent = otherEntity.GetComponent<TaskComponent>())
	{
		if (otherEntityTaskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			return false;
		}
	}

	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
	{
		if (resourceComponent->m_currentResources.m_resourceQuantities[i] > 0 && otherEntityResourceComponent->m_currentResources.m_resourceQuantities[i] > 0)
		{
			return true;
		}
	}

	return false;
}

bool ResourceSystems::CanEntityAffordTeamResourceChange(ArgusEntity entity, const FResourceSet& resourceChange)
{
	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForEntity(entity);
	if (!teamResourceComponent)
	{
		return false;
	}

	return teamResourceComponent->m_currentResources.CanAffordResourceChange(resourceChange);
}

bool ResourceSystems::ApplyTeamResourceChangeIfAffordable(ArgusEntity entity, const FResourceSet& resourceChange)
{
	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForEntity(entity);
	if (!teamResourceComponent)
	{
		return false;
	}

	if (!teamResourceComponent->m_currentResources.CanAffordResourceChange(resourceChange))
	{
		return false;
	}

	teamResourceComponent->m_currentResources.ApplyResourceChange(resourceChange);
	return true;
}

bool ResourceSystems::ApplyTeamResourceChangeIfAffordable(ETeam team, const FResourceSet& resourceChange)
{
	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForTeam(team);
	if (!teamResourceComponent)
	{
		return false;
	}

	if (!teamResourceComponent->m_currentResources.CanAffordResourceChange(resourceChange))
	{
		return false;
	}

	teamResourceComponent->m_currentResources.ApplyResourceChange(resourceChange);
	return true;
}

ResourceComponent* ResourceSystems::GetTeamResourceComponentForEntity(ArgusEntity entity)
{
	if (!entity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return nullptr;
	}

	IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return nullptr;
	}

	return GetTeamResourceComponentForTeam(identityComponent->m_team);
}

ResourceComponent* ResourceSystems::GetTeamResourceComponentForTeam(ETeam team)
{
	ArgusEntity teamEntity = ArgusEntity::GetTeamEntity(team);
	if (!teamEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::GetTeamEntity));
		return nullptr;
	}

	return teamEntity.GetComponent<ResourceComponent>();
}

void ResourceSystems::TransferResourcesBetweenComponents(ResourceComponent* sourceComponent, ResourceComponent* targetComponent, const FResourceSet& amount, const UResourceSetRecord* resourceCapacityRecord)
{
	if (!sourceComponent || !targetComponent)
	{
		return;
	}

	const FResourceSet* resourceCapacityPointer = nullptr;
	if (resourceCapacityRecord)
	{
		resourceCapacityPointer = &resourceCapacityRecord->m_resourceSet;
	}

	FResourceSet potentialResourceChange = sourceComponent->m_currentResources.CalculateResourceChangeAffordable(-amount);
	potentialResourceChange = targetComponent->m_currentResources.CalculateResourceChangeAffordable(-potentialResourceChange, resourceCapacityPointer);

	sourceComponent->m_currentResources.ApplyResourceChange(-potentialResourceChange);
	targetComponent->m_currentResources.ApplyResourceChange(potentialResourceChange);
}

void ResourceSystems::ClearResourceGatheringForEntity(const ResourceSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_targetingComponent->Reset();
	components.m_taskComponent->m_movementState = EMovementState::None;
	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::None;
}
