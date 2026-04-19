// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

struct ResourceExtractionComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_STATIC_DATA(UResourceSetRecord)
	uint32 m_resourcesToExtractRecordId = 0u;

	float m_extractionLengthSeconds = 1.0f;

	ARGUS_COMP_NO_DATA
	uint16 m_lastExtractionSourceEntityId = ArgusECSConstants::k_maxEntities;

	ARGUS_COMP_NO_DATA
	TimerHandle m_resourceExtractionTimer;
};