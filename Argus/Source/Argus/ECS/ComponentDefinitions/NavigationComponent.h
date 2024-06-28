// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include <vector>

struct NavigationComponent
{
	ARGUS_IGNORE()
	std::vector<FVector> m_navigationPoints = std::vector<FVector>();

	float  m_navigationSpeedUnitsPerSecond = 100.0f;

	ARGUS_IGNORE()
	uint16 m_lastPointIndex = 0u;

	void ResetPath()
	{
		m_navigationPoints.clear();
		m_lastPointIndex = 0u;
	}
};