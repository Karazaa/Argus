// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDataAssetComponentCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ArgusDataAssetComponentCodeGenerator::s_componentDataDirectorySuffix = "Source/Argus/ECS/DataComponentDefinitions/";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderTemplateFilename = "ComponentDataHeaderTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppTemplateFilename = "ComponentDataCppTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderSuffix = "Data.h";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppSuffix = "Data.cpp";
const char* ArgusDataAssetComponentCodeGenerator::s_propertyMacro = "\tUPROPERTY(EditAnywhere)";
const char* ArgusDataAssetComponentCodeGenerator::s_upropertyPrefix = "UPROPERTY";
const char* ArgusDataAssetComponentCodeGenerator::s_dataAssetComponentTemplateDirectorySuffix = "ComponentDataAssets/";

void ArgusDataAssetComponentCodeGenerator::GenerateDataAssetComponentsCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS static data asset code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Construct a directory path to data asset component templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_dataAssetComponentTemplateDirectorySuffix));

	std::string componentDataHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentDataHeaderTemplateFilename);
	std::string componentDataCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentDataCppTemplateFilename);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component static data asset code."), ARGUS_FUNCNAME)

	ArgusCodeGeneratorUtil::CombinedComponentDataOutput combinedComponentData;
	ArgusCodeGeneratorUtil::CombineStaticAndDynamicComponentData(parsedComponentData, combinedComponentData);

	// Parse header file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedHeaderFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseDataAssetHeaderFileTemplateWithReplacements(combinedComponentData, componentDataHeaderTemplateFilePath, outParsedHeaderFileContents);

	// Parse cpp file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseDataAssetCppFileTemplateWithReplacements(combinedComponentData, componentDataCppTemplateFilePath, outParsedCppFileContents);

	// Construct a directory path to registry location and to tests location. 
	FString componentDataDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentDataDirectory.Append(s_componentDataDirectorySuffix);
	FPaths::MakeStandardFilename(componentDataDirectory);
	const char* cStrComponentDataDirectory = ARGUS_FSTRING_TO_CHAR(componentDataDirectory);

	// Write out header and cpp files.
	for (int i = 0; i < outParsedHeaderFileContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentDataDirectory).append(outParsedHeaderFileContents[i].m_filename), outParsedHeaderFileContents[i].m_lines);
	}
	for (int i = 0; i < outParsedCppFileContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentDataDirectory).append(outParsedCppFileContents[i].m_filename), outParsedCppFileContents[i].m_lines);
	}

	if (didSucceed)
	{
		DeleteObsoleteFiles(combinedComponentData, cStrComponentDataDirectory);
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS static data asset code."), ARGUS_FUNCNAME)
	}
}

bool ArgusDataAssetComponentCodeGenerator::ParseDataAssetHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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
		ArgusCodeGeneratorUtil::FileWriteData writeData;
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
				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					std::string variable = "";
					if (!parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.empty())
					{
						const size_t propertyIgnoreDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyIgnoreDelimiter);
						const size_t propertyStaticDataDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter);

						if (propertyStaticDataDelimiterIndex != std::string::npos)
						{
							const size_t lineSize = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.length();
							const size_t startIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find('(') + 1;
							variable = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.substr(startIndex, (lineSize - 1) - startIndex);
							outParsedFileContents[i].m_lines.push_back(s_propertyMacro);
						}
						else if (propertyIgnoreDelimiterIndex == std::string::npos)
						{
							std::string overridePropertyText = std::regex_replace(parsedComponentData.m_componentVariableData[i][j].m_propertyMacro, std::regex(ArgusCodeGeneratorUtil::s_propertyDelimiter), s_upropertyPrefix);
							outParsedFileContents[i].m_lines.push_back(overridePropertyText);
						}
					}
					else
					{
						outParsedFileContents[i].m_lines.push_back(s_propertyMacro);
					}

					if (variable.empty())
					{
						variable = parsedComponentData.m_componentVariableData[i][j].m_typeName;
						variable.append(" ");
						variable.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
						if (!parsedComponentData.m_componentVariableData[i][j].m_defaultValue.empty())
						{
							variable.append(" = ");
							variable.append(parsedComponentData.m_componentVariableData[i][j].m_defaultValue);
						}
					}
					else
					{
						variable = std::regex_replace("\tTSoftObjectPtr<XXXXX>", std::regex("XXXXX"), variable);
						variable.append(" ");
						variable.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
					}
					variable.append(";");
					outParsedFileContents[i].m_lines.push_back(variable);
				}
			}
		}
		else if (headerLineText.find("$$$$$") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentDataAssetIncludeStatements.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(parsedComponentData.m_componentDataAssetIncludeStatements[i]);
			}
		}
		else if (headerLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				std::string perComponentHeaderLineText = headerLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentHeaderLineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
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

bool ArgusDataAssetComponentCodeGenerator::ParseDataAssetCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inCppStream = std::ifstream(templateFilePath);
	const FString ueCppFilePath = FString(templateFilePath.c_str());
	if (!inCppStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueCppFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		writeData.m_filename = parsedComponentData.m_componentNames[i];
		writeData.m_filename.append(s_componentDataCppSuffix);
		outParsedFileContents.push_back(writeData);
	}

	std::string cppLineText;
	while (std::getline(inCppStream, cppLineText))
	{
		if (cppLineText.find("@@@@@") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					std::string variableAssignment = "\t";
					variableAssignment.append(parsedComponentData.m_componentNames[i]);
					variableAssignment.append("Ref->");
					variableAssignment.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
					variableAssignment.append(" = ");

					const size_t propertyStaticDataDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter);
					if (propertyStaticDataDelimiterIndex != std::string::npos)
					{
						std::string staticDataStatement = std::regex_replace("ArgusStaticData::GetIdFromRecordSoftPtr(XXXXX)", std::regex("XXXXX"), parsedComponentData.m_componentVariableData[i][j].m_varName);
						variableAssignment.append(staticDataStatement);
					}
					else
					{
						variableAssignment.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
					}
					variableAssignment.append(";");

					outParsedFileContents[i].m_lines.push_back(variableAssignment);
				}
			}
		}
		else if (cppLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				std::string perComponentCppLineText = cppLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentCppLineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
		}
		else
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(cppLineText);
			}
		}
	}
	inCppStream.close();

	return true;
}

void ArgusDataAssetComponentCodeGenerator::DeleteObsoleteFiles(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, const char* componentDataDirectory)
{
	const std::string finalizedComponentDataDefinitionsDirectory = std::string(componentDataDirectory);
	std::vector<std::string> filesToDelete = std::vector<std::string>();

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDataDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		bool foundValidComponentName = false;
		for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
		{
			const size_t componentNameIndex = filePath.find(parsedComponentData.m_componentNames[i]);
			const size_t componentDataIndex = filePath.find("/ComponentData.h");
			if (componentNameIndex != std::string::npos || componentDataIndex != std::string::npos)
			{
				foundValidComponentName = true;
				break;
			}
		}

		if (foundValidComponentName)
		{
			continue;
		}

		filesToDelete.push_back(filePath);
	}

	for (int i = 0; i < filesToDelete.size(); ++i)
	{
		std::remove(filesToDelete[i].c_str());
	}
}
