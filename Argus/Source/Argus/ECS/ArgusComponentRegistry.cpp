// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ArgusComponentRegistry.h"

// Begin component specific definitions
#pragma region HealthComponent
HealthComponent ArgusComponentRegistry::s_HealthComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isHealthComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region IdentityComponent
IdentityComponent ArgusComponentRegistry::s_IdentityComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isIdentityComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region NavigationComponent
NavigationComponent ArgusComponentRegistry::s_NavigationComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isNavigationComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TargetingComponent
TargetingComponent ArgusComponentRegistry::s_TargetingComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTargetingComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TaskComponent
TaskComponent ArgusComponentRegistry::s_TaskComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTaskComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion
#pragma region TransformComponent
TransformComponent ArgusComponentRegistry::s_TransformComponents[ArgusECSConstants::k_maxEntities];
std::bitset<ArgusECSConstants::k_maxEntities> ArgusComponentRegistry::s_isTransformComponentActive = std::bitset<ArgusECSConstants::k_maxEntities>();
#pragma endregion

void ArgusComponentRegistry::FlushAllComponents()
{
	// Begin flush active component bitsets
	s_isHealthComponentActive.reset();
	s_isIdentityComponentActive.reset();
	s_isNavigationComponentActive.reset();
	s_isTargetingComponentActive.reset();
	s_isTaskComponentActive.reset();
	s_isTransformComponentActive.reset();

	// Begin flush component values
	for (int i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		s_HealthComponents[i] = HealthComponent();
		s_IdentityComponents[i] = IdentityComponent();
		s_NavigationComponents[i] = NavigationComponent();
		s_TargetingComponents[i] = TargetingComponent();
		s_TaskComponents[i] = TaskComponent();
		s_TransformComponents[i] = TransformComponent();
	}
}
