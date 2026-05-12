// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AssetFactoryGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* AssetFactoryGenerator::s_componentAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/ComponentDataFactories/";
const char* AssetFactoryGenerator::s_recordAssetFactoriesDirectorySuffix = "Source/ArgusEditor/AssetFactories/StaticRecordFactories/";
const char* AssetFactoryGenerator::s_argusEditorDirectorySuffix = "Source/ArgusEditor/";
const char* AssetFactoryGenerator::s_componentAssetFactoriesHeaderTemplateFilename = "ComponentAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesHeaderTemplateFilename = "RecordAssetFactoryHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_componentAssetFactoriesCppTemplateFilename = "ComponentAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_recordAssetFactoriesCppTemplateFilename = "RecordAssetFactoryCppTemplate.txt";
const char* AssetFactoryGenerator::s_argusEditorGeneratedHeaderTemplateFilename = "ArgusEditorGeneratedLogicHeaderTemplate.txt";
const char* AssetFactoryGenerator::s_argusEditorGeneratedCppTemplateFilename = "ArgusEditorGeneratedLogicCppTemplate.txt";
const char* AssetFactoryGenerator::s_assetActionsPerComponentTemplateFilename = "AssetActionsPerComponentFactoryTemplate.txt";
const char* AssetFactoryGenerator::s_assetActionsPerRecordTemplateFilename = "AssetActionsPerRecordTemplate.txt";
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

	didSucceed &= ParseComponentAssetFactoryTemplateFile(parsedComponentData, componentHeaderTemplateFilePath, true, componentAssetFactoryHeaders);
	didSucceed &= ParseComponentAssetFactoryTemplateFile(parsedComponentData, componentCppTemplateFilePath, false, componentAssetFactoryCpps);
	didSucceed &= ParseRecordAssetFactoryTemplateFile(parsedStaticDataRecords, recordHeaderTemplateFilePath, true, recordAssetFactoryHeaders);
	didSucceed &= ParseRecordAssetFactoryTemplateFile(parsedStaticDataRecords, recordCppTemplateFilePath, false, recordAssetFactoryCpps);

	FString componentAssetFactoriesDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentAssetFactoriesDirectory.Append(s_componentAssetFactoriesDirectorySuffix);
	FPaths::MakeStandardFilename(componentAssetFactoriesDirectory);

	FString recordAssetFactoriesDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	recordAssetFactoriesDirectory.Append(s_recordAssetFactoriesDirectorySuffix);
	FPaths::MakeStandardFilename(recordAssetFactoriesDirectory);

	FString argusEditorDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	argusEditorDirectory.Append(s_argusEditorDirectorySuffix);
	FPaths::MakeStandardFilename(argusEditorDirectory);

	const char* cStrComponentAssetFactoriesDirectory = ARGUS_FSTRING_TO_CHAR(componentAssetFactoriesDirectory);
	const char* cStrRecordAssetFactoriesDirectory = ARGUS_FSTRING_TO_CHAR(recordAssetFactoriesDirectory);
	const char* cStrArgusEditorDirectory = ARGUS_FSTRING_TO_CHAR(argusEditorDirectory);

	// Write out cpp files.
	for (int i = 0; i < componentAssetFactoryHeaders.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentAssetFactoriesDirectory).append(componentAssetFactoryHeaders[i].m_filename), componentAssetFactoryHeaders[i].m_lines);
	}
	for (int i = 0; i < componentAssetFactoryCpps.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentAssetFactoriesDirectory).append(componentAssetFactoryCpps[i].m_filename), componentAssetFactoryCpps[i].m_lines);
	}
	for (int i = 0; i < recordAssetFactoryHeaders.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRecordAssetFactoriesDirectory).append(recordAssetFactoryHeaders[i].m_filename), recordAssetFactoryHeaders[i].m_lines);
	}
	for (int i = 0; i < recordAssetFactoryCpps.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRecordAssetFactoriesDirectory).append(recordAssetFactoryCpps[i].m_filename), recordAssetFactoryCpps[i].m_lines);
	}

	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out asset factory code."), ARGUS_FUNCNAME)
	}
}

bool AssetFactoryGenerator::ParseComponentAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const std::string& templateFilePath, bool isHeader, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateFilePath);
	const FString ueTemplateFilePath = FString(templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData fileWriteData;
		fileWriteData.m_filename = parsedComponentData.m_componentNames[i];
		fileWriteData.m_filename.append("DataFactory");
		fileWriteData.m_filename.append(isHeader ? ".h" : ".cpp");
		outParsedFileContents.push_back(fileWriteData);
	}
	for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData fileWriteData;
		fileWriteData.m_filename = parsedComponentData.m_dynamicAllocComponentNames[i];
		fileWriteData.m_filename.append("DataFactory");
		fileWriteData.m_filename.append(isHeader ? ".h" : ".cpp");
		outParsedFileContents.push_back(fileWriteData);
	}

	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		if (templateLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				std::string perArgLineText = templateLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perArgLineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				int index = parsedComponentData.m_componentNames.size() + i;
				std::string perArgLineText = templateLineText;
				outParsedFileContents[index].m_lines.push_back(std::regex_replace(perArgLineText, std::regex("#####"), parsedComponentData.m_dynamicAllocComponentNames[i]));
			}
		}
		else
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(templateLineText);
			}
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				outParsedFileContents[parsedComponentData.m_componentNames.size() + i].m_lines.push_back(templateLineText);
			}
		}
	}

	inTemplateStream.close();
	return true;
}

bool AssetFactoryGenerator::ParseRecordAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const std::string& templateFilePath, bool isHeader, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateFilePath);
	const FString ueTemplateFilePath = FString(templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData fileWriteData;
		fileWriteData.m_filename = parsedStaticDataRecords.m_staticDataRecordNames[i].substr(1, parsedStaticDataRecords.m_staticDataRecordNames[i].length() - 1);
		fileWriteData.m_filename.append("Factory");
		fileWriteData.m_filename.append(isHeader ? ".h" : ".cpp");
		outParsedFileContents.push_back(fileWriteData);
	}

	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		if (templateLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
			{
				std::string perArgLineText = templateLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perArgLineText, std::regex("#####"), parsedStaticDataRecords.m_staticDataRecordNames[i].substr(1, parsedStaticDataRecords.m_staticDataRecordNames[i].length() - 1)));
			}
		}
		else
		{
			for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(templateLineText);
			}
		}
	}

	inTemplateStream.close();
	return true;
}