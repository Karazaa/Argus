// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct HealthComponent
{
	ARGUS_COMPONENT_SHARED;

	uint32 m_currentHealth = 1000u;
	uint32 m_maximumHealth = 1000u;
};