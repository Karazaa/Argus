// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

class TargetingSystems
{
public:
	static TOptional<FVector> GetCurrentTargetLocationForEntity(const ArgusEntity& entity);
	static bool IsInMeleeRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool IsInRangedRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
};