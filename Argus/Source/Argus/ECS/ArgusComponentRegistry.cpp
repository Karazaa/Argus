// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistry.h"

// HealthComponent
HealthComponent ArgusComponentRegistry::s_HealthComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isHealthComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();

void ArgusComponentRegistry::FlushAllComponents()
{
	s_isHealthComponentActive.reset();
	for (int i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_HealthComponents[i] = HealthComponent();
	}
}