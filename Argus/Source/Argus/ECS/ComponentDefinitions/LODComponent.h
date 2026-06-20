// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct LODComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	bool m_bIsInViewFrustrum = true;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	bool m_bWasInViewFrustrum = true;

	bool DidInViewFrustrumStatusChange() const
	{
		return m_bIsInViewFrustrum != m_bWasInViewFrustrum;
	}

	void PreInViewFrustrumUpdate()
	{
		m_bWasInViewFrustrum = m_bIsInViewFrustrum;
	}
};
