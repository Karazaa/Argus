// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "../ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentComponentSizeTest, "Argus.ECS.Component.ComponentSize", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentComponentSizeTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	size_t netSize = 0;

	// HealthComponent =================================================================================================
	const size_t HealthComponentSize = sizeof(HealthComponent);
	netSize += HealthComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), HealthComponentSize);
	// IdentityComponent =================================================================================================
	const size_t IdentityComponentSize = sizeof(IdentityComponent);
	netSize += IdentityComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), IdentityComponentSize);
	// NavigationComponent =================================================================================================
	const size_t NavigationComponentSize = sizeof(NavigationComponent);
	netSize += NavigationComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), NavigationComponentSize);
	// TargetingComponent =================================================================================================
	const size_t TargetingComponentSize = sizeof(TargetingComponent);
	netSize += TargetingComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent), TargetingComponentSize);
	// TransformComponent =================================================================================================
	const size_t TransformComponentSize = sizeof(TransformComponent);
	netSize += TransformComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), TransformComponentSize);

	netSize *= ArgusECSConstants::k_maxEntities;
	netSize /= 1024;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Net size of all component arrays = %d kb"), ARGUS_FUNCNAME, netSize);

	TestTrue(TEXT("Printing out component sizes"), true);
	return true;
}

#endif //WITH_AUTOMATION_TESTS
