// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TriggerSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "SystemArgumentDefinitions/TriggerSystemsArgs.h"

void TriggerSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TriggerSystems::RunSystems);

	ArgusIterators::IterateSystemsArgs<TriggerSystemsArgs>([](const TriggerSystemsArgs& components)
	{
		if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
		{
			return;
		}
		components.m_triggerComponent->m_removalStagedEntityIds.Reset();

		if (components.m_relativeCircleComponent)
		{
			UpdateCircleTriggerOverlaps(components);
			return;
		}
		
		if (components.m_relativePolygonComponent)
		{
			UpdatePolygonTriggerOverlaps(components);
			return;
		}

		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Tried to process trigger overlaps on entity without any relative geometry!"), ARGUS_FUNCNAME);
	});
}

void TriggerSystems::UpdateCircleTriggerOverlaps(const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::UpdateCircleTriggerOverlaps);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(components.m_relativeCircleComponent, ArgusECSLog);
}

void TriggerSystems::UpdatePolygonTriggerOverlaps(const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::UpdatePolygonTriggerOverlaps);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(components.m_relativePolygonComponent, ArgusECSLog);
}

void TriggerSystems::RegisterOverlappedEntityId(uint16 entityId, const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::RegisterOverlappedEntityId);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
}

void TriggerSystems::ProcessStagedRemovalEntityIds(const TriggerSystemsArgs& components)
{
	ARGUS_TRACE(TriggerSystems::ProcessStagedRemovalEntityIds);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
}