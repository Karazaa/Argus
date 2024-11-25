// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusTesting.h"
#include "ArgusEntity.h"

#if WITH_AUTOMATION_TESTS

bool ArgusTesting::s_isInTestingContext = false;

void ArgusTesting::StartArgusTest()
{
	s_isInTestingContext = true;
	ArgusEntity::FlushAllEntities();
}

void ArgusTesting::EndArgusTest()
{
	ArgusEntity::FlushAllEntities();
	s_isInTestingContext = false;
}

#endif //WITH_AUTOMATION_TESTS