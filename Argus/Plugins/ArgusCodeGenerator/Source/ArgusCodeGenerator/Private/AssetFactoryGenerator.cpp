// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AssetFactoryGenerator.h"

const char* AssetFactoryGenerator::s_componentAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/ComponentDataFactories/";
const char* AssetFactoryGenerator::s_recordAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/StaticRecordFactories/";
const char* AssetFactoryGenerator::s_componentAssetFactoriesHeaderTemplateFilename = "ComponentAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesHeaderTemplateFilename = "RecordAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_componentAssetFactoriesCppTemplateFilename = "ComponentAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesCppTemplateFilename = "RecordAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_assetFactoriesTemplateDirectorySuffix = "AssetFactories/";

void AssetFactoryGenerator::GenerateAssetFactories(ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords)
{

}
