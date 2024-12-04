// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

class UAbilityRecord;

struct AbilityComponent
{
	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability0Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability1Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability2Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability3Id = 0u;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d)\n    (%s: %d)\n    (%s: %d)\n    (%s: %d)"),
				ARGUS_NAMEOF(AbilityComponent),
				ARGUS_NAMEOF(m_ability0Id),
				m_ability0Id,
				ARGUS_NAMEOF(m_ability1Id),
				m_ability1Id,
				ARGUS_NAMEOF(m_ability2Id),
				m_ability2Id,
				ARGUS_NAMEOF(m_ability3Id),
				m_ability3Id
			)
		);
	}
};