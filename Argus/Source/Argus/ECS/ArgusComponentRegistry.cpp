// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ArgusComponentRegistry.h"

// Begin component specific definitions
// HealthComponent
HealthComponent ArgusComponentRegistry::s_HealthComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isHealthComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
// IdentityComponent
IdentityComponent ArgusComponentRegistry::s_IdentityComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isIdentityComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
// TargetingComponent
TargetingComponent ArgusComponentRegistry::s_TargetingComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTargetingComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
// TransformComponent
TransformComponent ArgusComponentRegistry::s_TransformComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTransformComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();

void ArgusComponentRegistry::FlushAllComponents()
{
	// Begin flush active component bitsets
	s_isHealthComponentActive.reset();
	s_isIdentityComponentActive.reset();
	s_isTargetingComponentActive.reset();
	s_isTransformComponentActive.reset();

	// Begin flush component values
	for (int i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_HealthComponents[i] = HealthComponent();
		s_IdentityComponents[i] = IdentityComponent();
		s_TargetingComponents[i] = TargetingComponent();
		s_TransformComponents[i] = TransformComponent();
	}
}
