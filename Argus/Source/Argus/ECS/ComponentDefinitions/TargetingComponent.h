// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"
#include "ArgusECSConstants.h"
#include "Misc/Optional.h"

struct TargetingComponent
{
	ARGUS_IGNORE()
	uint16 m_targetEntityId = ArgusECSConstants::k_maxEntities;

	ARGUS_IGNORE()
	TOptional<FVector> m_targetLocation = TOptional<FVector>();

	bool HasEntityTarget() const 
	{
		return m_targetEntityId < ArgusECSConstants::k_maxEntities; 
	}

	bool HasLocationTarget() const
	{
		return m_targetLocation.IsSet();
	}
};