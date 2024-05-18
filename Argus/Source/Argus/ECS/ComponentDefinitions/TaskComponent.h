// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"

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
};