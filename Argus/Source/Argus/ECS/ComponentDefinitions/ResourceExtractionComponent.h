// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

struct ResourceExtractionComponent
{
	ARGUS_COMPONENT_SHARED

	FResourceSet m_resourcesToExtract;
	float m_extractionLengthSeconds = 1.0f;

	ARGUS_IGNORE()
	TimerHandle m_resourceExtractionTimer;
};