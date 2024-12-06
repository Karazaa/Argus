// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

class ArgusTesting
{
public:
	static void StartArgusTest();
	static void EndArgusTest();
	static bool IsInTestingContext() { return s_isInTestingContext; }

private:
	static bool s_isInTestingContext;
};

#endif //WITH_AUTOMATION_TESTS