// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
class ARGUS_API ArgusECSCommandletInterface
{
public:
	static void InitializeECSForCommandlet();
	static void TeardownECSForCommandlet();
};
#endif //WITH_EDITOR