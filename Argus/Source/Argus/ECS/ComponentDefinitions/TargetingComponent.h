// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"
#include "ArgusECSConstants.h"

struct TargetingComponent
{
	uint32 m_targetEntityId = ArgusECSConstants::k_maxEntities;

	bool HasNoTarget() const { return m_targetEntityId >= ArgusECSConstants::k_maxEntities; }
};