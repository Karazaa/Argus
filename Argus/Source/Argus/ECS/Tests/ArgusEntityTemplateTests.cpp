// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "../ArgusEntityTemplate.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityTemplateInstantiateEntityTest, "Argus.ECS.EntityTemplate.InstantiateEntityTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityTemplateInstantiateEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	FString filePath = FPaths::ProjectContentDir();
	filePath.Append("ECSData/EntityTemplates/ExampleEntityTemplate.uasset");

	UArgusEntityTemplate* entityTemplate = LoadObject<UArgusEntityTemplate>(NULL, *filePath, NULL, LOAD_None, NULL);

	return true;
}

#endif //WITH_AUTOMATION_TESTS