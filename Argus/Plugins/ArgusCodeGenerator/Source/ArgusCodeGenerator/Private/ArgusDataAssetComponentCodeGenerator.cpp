// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDataAssetComponentCodeGenerator.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderTemplateFilename = "ComponentDataHeaderTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppTemplateFilename = "ComponentDataCppTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderSuffix = "Data.h";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppSuffix = "Data.cpp";

void ArgusDataAssetComponentCodeGenerator::GenerateDataAssetComponents(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS static data asset code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Construct a directory path to component templates
	const char* cStrTemplateDirectory = TCHAR_TO_UTF8(*ArgusCodeGeneratorUtil::GetTemplateDirectory());

	std::string componentDataHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentDataHeaderTemplateFilename);
	std::string componentDataCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentDataCppTemplateFilename);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate static data asset code."), ARGUS_FUNCNAME)

	// Parse header file
	std::vector<FileWriteData> outParsedHeaderFileContents = std::vector<FileWriteData>();
	didSucceed &= ParseDataAssetHeaderFileTemplateWithReplacements(parsedComponentData, componentDataHeaderTemplateFilePath, outParsedHeaderFileContents);

	// Parse cpp file
	std::vector<FileWriteData> outParsedCppFileContents = std::vector<FileWriteData>();
	didSucceed &= ParseDataAssetCppFileTemplateWithReplacements(parsedComponentData, componentDataCppTemplateFilePath, outParsedCppFileContents);

	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS static data asset code."), ARGUS_FUNCNAME)
	}
}

bool ArgusDataAssetComponentCodeGenerator::ParseDataAssetHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<FileWriteData>& outParsedFileContents)
{
	std::ifstream inHeaderStream = std::ifstream(templateFilePath);
	const FString ueHeaderFilePath = FString(templateFilePath.c_str());
	if (!inHeaderStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueHeaderFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		FileWriteData writeData;
		writeData.m_filename = parsedComponentData.m_componentNames[i];
		writeData.m_filename.append(s_componentDataHeaderSuffix);
		outParsedFileContents.push_back(writeData);
	}

	std::string headerLineText;
	while (std::getline(inHeaderStream, headerLineText))
	{
		if (headerLineText.find("@@@@@") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{

			}
		}
		else if (headerLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				std::string perComponentHeaderLinText = headerLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentHeaderLinText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
		}
		else
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(headerLineText);
			}
		}
	}
	inHeaderStream.close();

	return true;
}

bool ArgusDataAssetComponentCodeGenerator::ParseDataAssetCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<FileWriteData>& outParsedFileContents)
{

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{

	}
	return true;
}
