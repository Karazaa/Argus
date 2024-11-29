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
		// TODO JAMES: Fill out debug info for AbilityComponent.
	}
};