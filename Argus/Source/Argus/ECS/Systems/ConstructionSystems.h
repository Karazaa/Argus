// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class ConstructionSystems
{
public:
	static void RunSystems(float deltaTime);
	struct ConstructionSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		ConstructionComponent* m_constructionComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

	static bool CanEntityConstructOtherEntity(const ArgusEntity& potentialConstructor, const ArgusEntity& potentialConstructee);

private:
	static void ProcessBeingConstructedState(const ConstructionSystemsComponentArgs& components, float deltaTime);
	static void ProcessAutomaticConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime);
	static void ProcessManualConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime);
};