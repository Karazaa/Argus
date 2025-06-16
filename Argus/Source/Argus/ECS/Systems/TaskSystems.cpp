// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TaskSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void TaskSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TaskSystems::RunSystems);

	TaskSystemsArgs components;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(i)))
		{
			continue;
		}

		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			continue;
		}

		if (components.m_entity.IsIdle())
		{
			ProcessIdleEntity(components);
		}
	}
}

void TaskSystems::ProcessIdleEntity(const TaskSystemsArgs& components)
{

}
