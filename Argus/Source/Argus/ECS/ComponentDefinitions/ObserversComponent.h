// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentObservers/AbilityComponentObservers.h"
#include "ComponentObservers/PassengerComponentObservers.h"
#include "ComponentObservers/TaskComponentObservers.h"

struct ObserversComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_NO_DATA
	AbilityComponentObservers m_AbilityComponentObservers;

	ARGUS_NO_DATA
	PassengerComponentObservers m_PassengerComponentObservers;

	ARGUS_NO_DATA
	TaskComponentObservers m_TaskComponentObservers;
};