// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "DataComponentDefinitions/HealthComponentData.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityTemplateInstantiateEntityTest, "Argus.ECS.EntityTemplate.InstantiateEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityTemplateInstantiateEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedHealthValue = 5000u;

	ArgusEntity::FlushAllEntities();

	const uint16 targetId = static_cast<uint16>(UEntityPriority::MediumPriority);

	TestFalse
	(
		FString::Printf(TEXT("Validating that there isn't an entity yet.")),
		ArgusEntity::DoesEntityExist(targetId)
	);

	UHealthComponentData* healthComponentData = NewObject<UHealthComponentData>();
	healthComponentData->m_health = expectedHealthValue;

	UArgusEntityTemplate* entityTemplate = NewObject<UArgusEntityTemplate>();
	entityTemplate->m_entityPriority = UEntityPriority::MediumPriority;
	entityTemplate->m_componentData.Add(healthComponentData);

	ArgusEntity newEntity = entityTemplate->MakeEntity();

	TestTrue
	(
		FString::Printf(TEXT("[%s] Validating that an %s has been created."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		ArgusEntity::DoesEntityExist(targetId)
	);

	uint32 healthValue = newEntity.GetComponent<HealthComponent>()->m_health;

	TestEqual
	(
		FString::Printf(TEXT("[%s] Validating that a %s exists with the proper value, %d."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedHealthValue),
		healthValue,
		expectedHealthValue
	);

	return true;
}

#endif //WITH_AUTOMATION_TESTS