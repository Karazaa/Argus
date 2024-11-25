// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplate.h"
#include "ArgusTesting.h"
#include "DataComponentDefinitions/HealthComponentData.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityTemplateInstantiateEntityTest, "Argus.ECS.EntityTemplate.InstantiateEntity", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityTemplateInstantiateEntityTest::RunTest(const FString& Parameters)
{
	const uint32 expectedHealthValue = 5000u;
	const uint16 targetId = static_cast<uint16>(UEntityPriority::MediumPriority);

	ArgusTesting::StartArgusTest();

#pragma region Test that there is not an entity of a specific entityID
	TestFalse
	(
		FString::Printf(TEXT("[%s] Validating that there isn't an entity yet."), ARGUS_FUNCNAME),
		ArgusEntity::DoesEntityExist(targetId)
	);
#pragma endregion

	UHealthComponentData* healthComponentData = NewObject<UHealthComponentData>();
	healthComponentData->m_health = expectedHealthValue;

	UArgusEntityTemplate* entityTemplate = NewObject<UArgusEntityTemplate>();
	entityTemplate->m_entityPriority = UEntityPriority::MediumPriority;
	entityTemplate->m_componentData.Add(healthComponentData);

	ArgusEntity newEntity = entityTemplate->MakeEntity();

#pragma region Test that an entity has been successfully created from an entity template.
	TestTrue
	(
		FString::Printf(TEXT("[%s] Validating that an %s has been created."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)),
		ArgusEntity::DoesEntityExist(targetId)
	);
#pragma endregion

	uint32 healthValue = newEntity.GetComponent<HealthComponent>()->m_health;

#pragma region Test that a health component exists on the new entity with a specific health value.
	TestEqual
	(
		FString::Printf(TEXT("[%s] Validating that a %s exists with the proper value, %d."), ARGUS_FUNCNAME, ARGUS_NAMEOF(HealthComponent), expectedHealthValue),
		healthValue,
		expectedHealthValue
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS