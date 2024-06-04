// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

UENUM()
enum class ETask : uint8
{
	None,
	ProcessMoveToLocationCommand,
	ProcessMoveToEntityCommand,
	MoveToLocation,
	MoveToEntity,
	FailedToFindPath,
};

struct TaskComponent
{
	ETask m_currentTask = ETask::None;

	bool IsExecutingMoveTask() const
	{
		return m_currentTask == ETask::MoveToLocation || m_currentTask == ETask::MoveToEntity;
	}
};