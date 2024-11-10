// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

class UArgusActorRecord;

struct SpawningComponent
{
	ARGUS_STATIC_DATA(UArgusActorRecord)
	uint32 m_argusActorRecordId = 0u;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s]\n    (%s: %d)"),
				ARGUS_NAMEOF(SpawningComponent),
				ARGUS_NAMEOF(m_argusActorRecordId),
				m_argusActorRecordId
			)
		);
	}
};