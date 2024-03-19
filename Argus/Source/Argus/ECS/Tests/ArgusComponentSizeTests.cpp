// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntity.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentComponentSizeTest, "Argus.ECS.Component.ComponentSizeTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentComponentSizeTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	size_t netSize = 0;
	const size_t healthComponentSize = sizeof(HealthComponent);
	netSize += healthComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), healthComponentSize);

	const size_t transformComponentSize = sizeof(TransformComponent);
	netSize += transformComponentSize;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), transformComponentSize);

	netSize *= ArgusECSConstants::k_maxEntities;
	netSize /= 1024;
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] Net size of all component arrays = %d"), ARGUS_FUNCNAME, netSize);

	TestTrue(TEXT("Printing out component sizes"), true);
	return true;
}

#endif //WITH_AUTOMATION_TESTS