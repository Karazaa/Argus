// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/Timer.h"
#include <vector>

struct TimerComponent
{
	ARGUS_IGNORE()
	std::vector<Timer> m_timers = std::vector<Timer>();

	void GetDebugString(FString& debugStringToAppendTo) const
	{

	}
};