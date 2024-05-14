// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"
#include <vector>

struct NavigationComponent
{
	ARGUS_IGNORE()
	std::vector<FVector> m_navigationPoints = std::vector<FVector>();
	ARGUS_IGNORE()
	uint32 m_lastPointIndex = 0u;
	float  m_navigationSpeedUnitsPerSecond = 100.0f;
};