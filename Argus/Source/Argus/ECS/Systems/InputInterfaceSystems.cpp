// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "InputInterfaceSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void InputInterfaceSystems::CheckAndHandleEntityDoubleClick(const ArgusEntity& entity)
{
	if (!entity)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	if (inputInterfaceComponent->m_doubleClickTimer.IsTimerTicking(singletonEntity))
	{
		if (entity.GetId() == inputInterfaceComponent->m_lastSelectedEntityId)
		{
			inputInterfaceComponent->m_lastSelectedEntityId = ArgusECSConstants::k_maxEntities;
			inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);

			// TODO JAMES: Valid double click! do something.
			return;
		}
	}

	inputInterfaceComponent->m_lastSelectedEntityId = entity.GetId();
	inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);
	inputInterfaceComponent->m_doubleClickTimer.StartTimer(singletonEntity, inputInterfaceComponent->m_doubleClickThresholdSeconds);
}
