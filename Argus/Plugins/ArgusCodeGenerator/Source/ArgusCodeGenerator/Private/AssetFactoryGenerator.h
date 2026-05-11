// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class AssetFactoryGenerator
{
public:
	static void GenerateAssetFactories(ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords);

private:
	static const char* s_componentAssetFactoriesDirectorySuffix;
	static const char* s_recordAssetFactoriesDirectorySuffix;
	static const char* s_componentAssetFactoriesHeaderTemplateFilename;
	static const char* s_recordAssetFactoriesHeaderTemplateFilename;
	static const char* s_componentAssetFactoriesCppTemplateFilename;
	static const char* s_recordAssetFactoriesCppTemplateFilename;
	static const char* s_assetFactoriesTemplateDirectorySuffix;
};