// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TargetingSystemsArgs.h"

class TargetingSystems
{
public:
	static void RunSystems(float deltaTime);

	static TOptional<FVector> GetCurrentTargetLocationForEntity(const ArgusEntity& entity);
	static bool IsInMeleeRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool IsInRangedRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);

private:
	static void ProcessIdleEntity(const TargetingSystemsArgs& components);
};