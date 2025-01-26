// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusComponentComponentSizeTest, "Argus.ECS.Component.ComponentSize", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusComponentComponentSizeTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();

	size_t netSize = 0;

#pragma region AbilityComponent
	const size_t AbilityComponentSize = sizeof(AbilityComponent);
	netSize += AbilityComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(AbilityComponent), AbilityComponentSize);
#pragma endregion
#pragma region ConstructionComponent
	const size_t ConstructionComponentSize = sizeof(ConstructionComponent);
	netSize += ConstructionComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ConstructionComponent), ConstructionComponentSize);
#pragma endregion
#pragma region HealthComponent
	const size_t HealthComponentSize = sizeof(HealthComponent);
	netSize += HealthComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), HealthComponentSize);
#pragma endregion
#pragma region IdentityComponent
	const size_t IdentityComponentSize = sizeof(IdentityComponent);
	netSize += IdentityComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), IdentityComponentSize);
#pragma endregion
#pragma region NavigationComponent
	const size_t NavigationComponentSize = sizeof(NavigationComponent);
	netSize += NavigationComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), NavigationComponentSize);
#pragma endregion
#pragma region SpawningComponent
	const size_t SpawningComponentSize = sizeof(SpawningComponent);
	netSize += SpawningComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawningComponent), SpawningComponentSize);
#pragma endregion
#pragma region TargetingComponent
	const size_t TargetingComponentSize = sizeof(TargetingComponent);
	netSize += TargetingComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent), TargetingComponentSize);
#pragma endregion
#pragma region TaskComponent
	const size_t TaskComponentSize = sizeof(TaskComponent);
	netSize += TaskComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TaskComponent), TaskComponentSize);
#pragma endregion
#pragma region TimerComponent
	const size_t TimerComponentSize = sizeof(TimerComponent);
	netSize += TimerComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TimerComponent), TimerComponentSize);
#pragma endregion
#pragma region TransformComponent
	const size_t TransformComponentSize = sizeof(TransformComponent);
	netSize += TransformComponentSize;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Size of %s = %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), TransformComponentSize);
#pragma endregion

	netSize *= ArgusECSConstants::k_maxEntities;
	netSize /= 1024;
	ARGUS_LOG(ArgusTestingLog, Display, TEXT("[%s] Net size of all component arrays = %d kb"), ARGUS_FUNCNAME, netSize);

	TestTrue(TEXT("Printing out component sizes"), true);

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS
