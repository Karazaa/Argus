// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentObservers/AbilityComponentObservers.h"
#include "ComponentObservers/PassengerComponentObservers.h"
#include "ComponentObservers/TaskComponentObservers.h"

struct ObserversComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	AbilityComponentObservers m_AbilityComponentObservers;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	PassengerComponentObservers m_PassengerComponentObservers;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TaskComponentObservers m_TaskComponentObservers;
};