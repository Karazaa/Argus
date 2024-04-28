// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "DataComponentDefinitions/HealthComponentData.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityTemplateInstantiateEntityTest, "Argus.ECS.EntityTemplate.InstantiateEntityTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityTemplateInstantiateEntityTest::RunTest(const FString& Parameters)
{
	ArgusEntity::FlushAllEntities();

	const uint16 targetId = static_cast<uint16>(UEntityPriority::MediumPriority);
	TestFalse(TEXT("Validating that there isn't an entity yet."), ArgusEntity::DoesEntityExist(targetId));

	UHealthComponentData* healthComponentData = NewObject<UHealthComponentData>();
	healthComponentData->m_health = 5000u;

	UArgusEntityTemplate* entityTemplate = NewObject<UArgusEntityTemplate>();
	entityTemplate->EntityPriority = UEntityPriority::MediumPriority;
	entityTemplate->ComponentData.Add(healthComponentData);

	ArgusEntity newEntity = entityTemplate->MakeEntity();

	TestTrue(TEXT("Validating that an entity has been created"), ArgusEntity::DoesEntityExist(targetId));
	uint32 healthValue = newEntity.GetComponent<HealthComponent>()->m_health;

	TestEqual(TEXT("Validating that a health component exists with the proper value."), healthValue, 5000u);
	return true;
}

#endif //WITH_AUTOMATION_TESTS