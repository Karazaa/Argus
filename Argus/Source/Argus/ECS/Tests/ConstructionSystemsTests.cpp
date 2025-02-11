// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusTesting.h"
#include "Systems/ConstructionSystems.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ConstructionSystemsBeingConstructedAutomaticTest, "Argus.ECS.Systems.ConstructionSystems.BeingConstructed.Automatic", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ConstructionSystemsBeingConstructedAutomaticTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ConstructionSystemsBeingConstructedManualTest, "Argus.ECS.Systems.ConstructionSystems.BeingConstructed.Manual", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ConstructionSystemsBeingConstructedManualTest::RunTest(const FString& Parameters)
{
	ArgusTesting::StartArgusTest();

	ArgusTesting::EndArgusTest();
	return true;
}

#endif // WITH_AUTOMATION_TESTS