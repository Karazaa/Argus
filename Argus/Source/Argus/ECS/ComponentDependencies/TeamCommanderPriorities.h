// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ETeamCommanderDirective : uint8
{
	ExtractResources,
	Scout,

	Count
};

struct TeamCommanderPriority
{
	ETeamCommanderDirective m_directive = ETeamCommanderDirective::Scout;
	float m_weight = 0.0f;
};