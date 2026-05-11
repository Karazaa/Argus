// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AssetFactoryGenerator.h"

const char* AssetFactoryGenerator::s_componentAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/ComponentDataFactories/";
const char* AssetFactoryGenerator::s_recordAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/StaticRecordFactories/";
const char* AssetFactoryGenerator::s_componentAssetFactoriesHeaderTemplateFilename = "ComponentAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesHeaderTemplateFilename = "RecordAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_componentAssetFactoriesCppTemplateFilename = "ComponentAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesCppTemplateFilename = "RecordAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_assetFactoriesTemplateDirectorySuffix = "AssetFactories/";

void AssetFactoryGenerator::GenerateAssetFactories(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of asset factory code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate asset factory code."), ARGUS_FUNCNAME)

	// Construct a directory path to static data templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_assetFactoriesTemplateDirectorySuffix));
	
	std::string componentHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentAssetFactoriesHeaderTemplateFilename);
	std::string componentCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentAssetFactoriesCppTemplateFilename);
	std::string recordHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_recordAssetFactoriesHeaderTemplateFilename);
	std::string recordCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_recordAssetFactoriesCppTemplateFilename);

	std::vector<ArgusCodeGeneratorUtil::FileWriteData> componentAssetFactoryHeaders, componentAssetFactoryCpps, recordAssetFactoryHeaders, recordAssetFactoryCpps;
	int numComponents = parsedComponentData.m_componentNames.size() + parsedComponentData.m_dynamicAllocComponentNames.size();
	int numRecords = parsedStaticDataRecords.m_staticDataRecordNames.size();
	componentAssetFactoryHeaders.reserve(numComponents);
	componentAssetFactoryCpps.reserve(numComponents);
	recordAssetFactoryHeaders.reserve(numRecords);
	recordAssetFactoryCpps.reserve(numRecords);

	didSucceed &= ParseComponentAssetFactoryTemplateFile(parsedComponentData, componentHeaderTemplateFilePath, componentAssetFactoryHeaders);
	didSucceed &= ParseComponentAssetFactoryTemplateFile(parsedComponentData, componentCppTemplateFilePath, componentAssetFactoryCpps);
	didSucceed &= ParseRecordAssetFactoryTemplateFile(parsedStaticDataRecords, recordHeaderTemplateFilePath, recordAssetFactoryHeaders);
	didSucceed &= ParseRecordAssetFactoryTemplateFile(parsedStaticDataRecords, recordCppTemplateFilePath, recordAssetFactoryCpps);
}

bool AssetFactoryGenerator::ParseComponentAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	return true;
}

bool AssetFactoryGenerator::ParseRecordAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	return true;
}