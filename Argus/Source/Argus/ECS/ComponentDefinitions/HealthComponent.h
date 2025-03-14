// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct HealthComponent
{
	ARGUS_COMPONENT_SHARED

	uint32 m_currentHealth = 1000u;
	uint32 m_maximumHealth = 1000u;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d) \n    (%s: %d)"), 
				ARGUS_NAMEOF(HealthComponent), 
				ARGUS_NAMEOF(m_currentHealth),
				m_currentHealth,
				ARGUS_NAMEOF(m_maximumHealth),
				m_maximumHealth
			)
		);
	}
};