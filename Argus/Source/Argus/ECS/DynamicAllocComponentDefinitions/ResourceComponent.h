// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ResourceSet.h"
#include "CoreMinimal.h"

struct ResourceComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED

	FResourceSet m_resourceSet;
};