// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TriggerSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/RelativeGeometrySystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#include "DrawDebugHelpers.h"
#endif //!UE_BUILD_SHIPPING

void TriggerSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TriggerSystems::RunSystems);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	ArgusIterators::IterateSystemsArgs<TriggerSystemsArgs>([spatialPartitioningComponent](const TriggerSystemsArgs& components)
	{
		if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
		{
			return;
		}

		components.m_triggerComponent->m_removalStagedEntityIds.Reset();
		for (uint16 entityId : components.m_triggerComponent->m_overlappingEntityIds)
		{
			components.m_triggerComponent->m_removalStagedEntityIds.Add(entityId);
		}

		if (components.m_relativeCircleComponent)
		{
			UpdateCircleTriggerOverlaps(components, spatialPartitioningComponent);
			return;
		}
		
		if (components.m_relativePolygonComponent)
		{
			UpdatePolygonTriggerOverlaps(components, spatialPartitioningComponent);
			return;
		}

		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Tried to process trigger overlaps on entity without any relative geometry!"), ARGUS_FUNCNAME);
	});
}

void TriggerSystems::UpdateCircleTriggerOverlaps(const TriggerSystemsArgs& components, SpatialPartitioningComponent* spatialPartitioningComponent)
{
	ARGUS_TRACE(TriggerSystems::UpdateCircleTriggerOverlaps);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(components.m_relativeCircleComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	FVector center = RelativeGeometrySystems::GetWorldSpaceRelativeCircleCenter(components.m_relativeCircleComponent, components.m_facingComponent, components.m_transformComponent);
	const float radius = components.m_relativeCircleComponent->m_radius;

#if !UE_BUILD_SHIPPING
	if (WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>())
	{
		DrawDebugCircle(worldReferenceComponent->m_worldPointer, center, radius, 20, FColor::Green, false, -1.0f, 0, ArgusECSConstants::k_debugDrawLineWidth, FVector::RightVector, FVector::ForwardVector, false);
	}
#endif //!UE_BUILD_SHIPPING

	if (components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::BothGroundedAndFlying ||
		components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::OnlyGrounded)
	{
		const TArray<uint16, ArgusContainerAllocator<20u> >& foundEntityIds = spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(center, radius, components.m_entity);
		UpdateOverlappingEntities(foundEntityIds, components);
	}
	if (components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::BothGroundedAndFlying ||
		components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::OnlyFlying)
	{
		center.Z = spatialPartitioningComponent->m_flyingPlaneHeight;
		const TArray<uint16, ArgusContainerAllocator<20u> >& foundEntityIds = spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(center, radius, components.m_entity);
		UpdateOverlappingEntities(foundEntityIds, components);
	}
}

void TriggerSystems::UpdatePolygonTriggerOverlaps(const TriggerSystemsArgs& components, SpatialPartitioningComponent* spatialPartitioningComponent)
{
	ARGUS_TRACE(TriggerSystems::UpdatePolygonTriggerOverlaps);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(components.m_relativePolygonComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	TArray<FVector> polygonPoints;
	RelativeGeometrySystems::GetWorldSpaceRelativePolygonPoints(components.m_relativePolygonComponent, components.m_facingComponent, components.m_transformComponent, polygonPoints);

	if (components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::BothGroundedAndFlying ||
		components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::OnlyGrounded)
	{
		const TArray<uint16, ArgusContainerAllocator<20u> >& foundEntityIds = spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinConvexPoly(polygonPoints, components.m_entity);
		UpdateOverlappingEntities(foundEntityIds, components);
	}
	if (components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::BothGroundedAndFlying ||
		components.m_triggerComponent->m_triggerPlanarOverlaps == EFlightCapability::OnlyFlying)
	{
		// TODO JAMES: Adjust polygon point height to the flying plane height.
		const TArray<uint16, ArgusContainerAllocator<20u> >& foundEntityIds = spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindArgusEntityIdsWithinConvexPoly(polygonPoints, components.m_entity);
		UpdateOverlappingEntities(foundEntityIds, components);
	}
}

void TriggerSystems::RegisterOverlappedEntityId(uint16 entityId, const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::RegisterOverlappedEntityId);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	bool alreadyInSet = false;
	components.m_triggerComponent->m_overlappingEntityIds.FindOrAdd(entityId, &alreadyInSet);

	if (alreadyInSet)
	{
		components.m_triggerComponent->m_removalStagedEntityIds.Remove(entityId);
	}
	else
	{
		EnteredTriggerThisFrame(entityId, components);
	}
}

void TriggerSystems::ProcessStagedRemovalEntityIds(const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::ProcessStagedRemovalEntityIds);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	for (uint16 entityId : components.m_triggerComponent->m_removalStagedEntityIds)
	{
		components.m_triggerComponent->m_overlappingEntityIds.Remove(entityId);
		ExitedTriggerThisFrame(entityId, components);
	}

	components.m_triggerComponent->m_removalStagedEntityIds.Reset();
}

void TriggerSystems::EnteredTriggerThisFrame(uint16 entityId, const TriggerSystemsArgs& components)
{
}

void TriggerSystems::ExitedTriggerThisFrame(uint16 entityId, const TriggerSystemsArgs& components)
{
}