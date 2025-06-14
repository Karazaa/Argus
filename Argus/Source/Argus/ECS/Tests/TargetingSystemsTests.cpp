// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Systems/TargetingSystems.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TargetingSystemsTargetNearestEntityMatchingTeamMaskTest, "Argus.ECS.Systems.TargetingSystems.TargetNearestEntityMatchingTeamMask", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TargetingSystemsTargetNearestEntityMatchingTeamMaskTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();

	// TODO JAMES: Remake Targeting Systems Tests.

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS