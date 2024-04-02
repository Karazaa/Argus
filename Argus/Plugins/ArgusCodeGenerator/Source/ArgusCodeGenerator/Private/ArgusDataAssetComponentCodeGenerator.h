// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once
#include "ArgusCodeGeneratorUtil.h"

class ArgusDataAssetComponentCodeGenerator
{
public:
	static void GenerateDataAssetComponents(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);
private:
	static const char* s_componentDataHeaderTemplateFilename;
	static const char* s_componentDataCppTemplateFilename;
};