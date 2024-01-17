// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "ArgusEntity.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusEntityGetIdTest, "Argus.Entity.GetId", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusEntityGetIdTest::RunTest(const FString& Parameters)
{
	ArgusEntity entity = ArgusEntity(10u);
	TestEqual(TEXT("Testing ArgusEntity creation and ID storage."), entity.GetId(), 10u);
	
	return true;
}

#endif //WITH_AUTOMATION_TESTS
