// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusStaticDataCodeGenerator
{
public:
	static void GenerateStaticDataCode(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords);

private:
	static const char* s_staticDataTemplateDirectorySuffix;
};