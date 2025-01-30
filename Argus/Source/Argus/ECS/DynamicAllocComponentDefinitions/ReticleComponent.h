// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct ReticleComponent
{
	ARGUS_IGNORE()
	FVector m_reticleLocation = FVector::ZeroVector;

	ARGUS_IGNORE()
	uint32 m_abilityRecordId = 0u;

	ARGUS_IGNORE()
	float m_radius = 100.0f;

	ARGUS_IGNORE()
	bool m_isBlocked = false;

	ARGUS_IGNORE()
	bool m_wasAbilityCast = false;

	void DisableReticle() 
	{ 
		m_abilityRecordId = 0u;
		m_wasAbilityCast = false;
	}

	bool IsReticleEnabled() const { return m_abilityRecordId != 0u; }

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