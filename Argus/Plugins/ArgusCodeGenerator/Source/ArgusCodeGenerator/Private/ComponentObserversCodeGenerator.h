// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ComponentObserversGenerator
{
public:
	static void GenerateComponentObserversCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponents);
};