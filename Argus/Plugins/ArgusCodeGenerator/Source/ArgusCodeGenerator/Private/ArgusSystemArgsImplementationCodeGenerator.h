// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusSystemArgsImplementationCodeGenerator
{
public:
	static void GenerateSystemArgsImplementation(const ArgusCodeGeneratorUtil::ParseSystemArgDefinitionsOutput& parsedSystemArgs);
};