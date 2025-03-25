// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "Misc/Optional.h"

struct TargetingComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	TOptional<FVector> m_targetLocation = TOptional<FVector>();

	float m_meleeRange = 400.0f;
	float m_rangedRange = 400.0f;

	ARGUS_IGNORE()
	uint16 m_targetEntityId = ArgusECSConstants::k_maxEntities;

	bool HasLocationTarget() const
	{
		return m_targetLocation.IsSet();
	}

	bool HasEntityTarget() const
	{
		return m_targetEntityId < ArgusECSConstants::k_maxEntities;
	}

	bool HasSameTarget(const TargetingComponent* other) const
	{
		if (!other)
		{
			return false;
		}

		if (HasLocationTarget() && other->HasLocationTarget())
		{
			return m_targetLocation.GetValue() == other->m_targetLocation.GetValue();
		}
		
		if (HasEntityTarget() && other->HasEntityTarget())
		{
			return m_targetEntityId == other->m_targetEntityId;
		}

		return false;
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %f) \n    (%s: %f)"), 
				ARGUS_NAMEOF(TargetingComponent),
				ARGUS_NAMEOF(m_meleeRange),
				m_meleeRange,
				ARGUS_NAMEOF(m_rangedRange),
				m_rangedRange
			)
		);
	}
};