// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentObservers/TaskComponentObservers.h"

struct ObserversComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	TaskComponentObservers m_TaskComponentObservers;
};