// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct ReticleComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	FVector m_reticleLocation = FVector::ZeroVector;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint32 m_abilityRecordId = 0u;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	float m_radius = 100.0f;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	bool m_isBlocked = false;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	bool m_wasAbilityCast = false;

	void DisableReticle() 
	{ 
		m_abilityRecordId = 0u;
		m_wasAbilityCast = false;
	}

	bool IsReticleEnabled() const { return m_abilityRecordId != 0u; }
};