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
	SpawnedWaitingForActorTake,
	DestroyedWaitingForActorRelease
};

struct TaskComponent
{
	ETask m_currentTask = ETask::None;

	bool IsExecutingMoveTask() const
	{
		return m_currentTask == ETask::MoveToLocation || m_currentTask == ETask::MoveToEntity;
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		auto taskName = TEXT("");
		switch (m_currentTask)
		{
			case ETask::None:
				taskName = ARGUS_NAMEOF(ETask::None);
				break;
			case ETask::ProcessMoveToLocationCommand:
				taskName = ARGUS_NAMEOF(ETask::ProcessMoveToLocationCommand);
				break;
			case ETask::ProcessMoveToEntityCommand:
				taskName = ARGUS_NAMEOF(ETask::ProcessMoveToEntityCommand);
				break;
			case ETask::MoveToLocation:
				taskName = ARGUS_NAMEOF(ETask::MoveToLocation);
				break;
			case ETask::MoveToEntity:
				taskName = ARGUS_NAMEOF(ETask::MoveToEntity);
				break;
			case ETask::FailedToFindPath:
				taskName = ARGUS_NAMEOF(ETask::FailedToFindPath);
			case ETask::SpawnedWaitingForActorTake:
				taskName = ARGUS_NAMEOF(ETask::SpawnedWaitingForActorTake);
				break;
			case ETask::DestroyedWaitingForActorRelease:
				taskName = ARGUS_NAMEOF(ETask::DestroyedWaitingForActorRelease);
				break;
		}

		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %s)"), 
				ARGUS_NAMEOF(TaskComponent),
				ARGUS_NAMEOF(m_currentTask),
				taskName
			)
		);
	}
};