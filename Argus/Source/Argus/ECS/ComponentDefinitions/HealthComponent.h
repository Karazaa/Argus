// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct HealthComponent
{
	uint32 m_health = 1000u;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d)"), 
				ARGUS_NAMEOF(HealthComponent), 
				ARGUS_NAMEOF(m_health), 
				m_health
			)
		);
	}
};