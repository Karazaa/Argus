// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TargetingSystems
{
public:
	static void RunSystems(float deltaTime);

	struct TargetingSystemsComponentArgs
	{
		TargetingComponent* m_targetingComponent = nullptr;
		const TransformComponent* m_transformComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};

	static void TargetNearestEntityMatchingTeamMask(uint16 sourceEntityID, uint8 teamMask, const TargetingSystemsComponentArgs& components);
	static TOptional<FVector> GetCurrentTargetLocationForEntity(const ArgusEntity& entity);
	static bool IsInMeleeRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
	static bool IsInRangedRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
};