// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TargetingSystems
{
public:
	static TOptional<FVector> GetCurrentTargetLocationForEntity(ArgusEntity entity);
	static bool IsInMeleeRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
	static bool IsInRangedRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
	static bool IsInSightRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
	static float GetRangeToUseForOtherEntity(ArgusEntity entity, ArgusEntity otherEntity);
};