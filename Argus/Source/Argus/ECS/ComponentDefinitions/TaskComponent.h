// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"

UENUM()
enum class ETask : uint8
{
	None = 0,
	MoveToLocation = 1,
	MoveToTarget = 2,
};

struct TaskComponent
{
	ETask m_currentTask = ETask::None;
};