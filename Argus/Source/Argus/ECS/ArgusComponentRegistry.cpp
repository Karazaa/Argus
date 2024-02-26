// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistry.h"

HealthComponent ArgusComponentRegistry::s_healthComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isHealthComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();

void ArgusComponentRegistry::FlushAllComponents()
{
	s_isHealthComponentActive.reset();
	for (int i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_healthComponents[i].Reset();
	}
}