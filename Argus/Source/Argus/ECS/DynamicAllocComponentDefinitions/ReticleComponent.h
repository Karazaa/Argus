// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct ReticleComponent
{
	ARGUS_IGNORE()
	uint32 m_abilityRecordId = 0u;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d)"),
				ARGUS_NAMEOF(ReticleComponent),
				ARGUS_NAMEOF(m_abilityRecordId),
				m_abilityRecordId
			)
		);
	}
};