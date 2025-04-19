// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ResourceSet.h"
#include "CoreMinimal.h"

class UResourceSetRecord;

struct ResourceComponent
{
	ARGUS_COMPONENT_SHARED

	FResourceSet m_currentResources;

	ARGUS_STATIC_DATA(UResourceSetRecord)
	uint32 m_resourceCapacityRecordId = 0u;

	bool m_isExtractionSource = false;
};