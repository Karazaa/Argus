// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TargetingSystemsArgs.h"

class TargetingSystems
{
public:
	static void RunSystems(float deltaTime);

	static void TargetNearestEntityMatchingTeamMask(uint16 sourceEntityID, uint8 teamMask, const TargetingSystemsArgs& components);
	static TOptional<FVector> GetCurrentTargetLocationForEntity(const ArgusEntity& entity);
	static bool IsInMeleeRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool IsInRangedRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
};