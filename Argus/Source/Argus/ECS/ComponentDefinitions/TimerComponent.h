// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/Timer.h"

struct TimerComponent
{
	ARGUS_IGNORE()
	Timer m_timer;

	void GetDebugString(FString& debugStringToAppendTo) const
	{

	}
};