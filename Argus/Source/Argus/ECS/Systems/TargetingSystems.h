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
};