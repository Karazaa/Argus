// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDataAssetComponentCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <unordered_set>

const char* ArgusDataAssetComponentCodeGenerator::s_componentDataDirectorySuffix = "Source/Argus/ECS/DataComponentDefinitions/";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderTemplateFilename = "ComponentDataHeaderTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppTemplateFilename = "ComponentDataCppTemplate.txt";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataHeaderSuffix = "Data.h";
const char* ArgusDataAssetComponentCodeGenerator::s_componentDataCppSuffix = "Data.cpp";
const char* ArgusDataAssetComponentCodeGenerator::s_propertyMacro = "\tUPROPERTY(EditAnywhere)";
const char* ArgusDataAssetComponentCodeGenerator::s_upropertyPrefix = "UPROPERTY";
const char* ArgusDataAssetComponentCodeGenerator::s_dataAssetComponentTemplateDirectorySuffix = "ComponentDataAssets/";
const char* ArgusDataAssetComponentCodeGenerator::s_exponentialDecaySmootherTypeName = "ArgusMath::ExponentialDecaySmoother";

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
		ArgusCodeGeneratorUtil::DeleteObsoleteComponentDependentFiles(combinedComponentData.m_componentNames, cStrComponentDataDirectory, "/ComponentData.h");
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
		if (headerLineText.find("%%%%%") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				std::unordered_set<std::string> addedIncludes;
				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					if (parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName.empty() || addedIncludes.contains(parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName))
					{
						continue;
					}

					std::string staticDataName = parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName;
					addedIncludes.insert(staticDataName);
					staticDataName = staticDataName.substr(1, staticDataName.length() - 1);
					outParsedFileContents[i].m_lines.push_back(std::vformat("#include \"RecordReferences/{}Reference.h\"", std::make_format_args(staticDataName)));
				}
			}
		}
		else if (headerLineText.find("@@@@@") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					std::string variable = "";
					if (!parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.empty())
					{
						const size_t propertyIgnoreDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyIgnoreDelimiter);
						const size_t propertyObservableDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyObservableDelimiter);
						const size_t propertyStaticDataDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter);

						if (propertyStaticDataDelimiterIndex != std::string::npos)
						{
							outParsedFileContents[i].m_lines.push_back(s_propertyMacro);
						}
						else if (propertyObservableDelimiterIndex != std::string::npos)
						{
							const size_t propertyObservablePropertyDeclarationDelimiter = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyObservablePropertyDeclarationDelimiter);
							
							if (propertyObservablePropertyDeclarationDelimiter != std::string::npos)
							{
								outParsedFileContents[i].m_lines.push_back(s_propertyMacro);
							}
							else
							{
								continue;
							}
						}
						else if (propertyIgnoreDelimiterIndex != std::string::npos)
						{
							continue;
						}
						else
						{
							std::string overridePropertyText = std::regex_replace(parsedComponentData.m_componentVariableData[i][j].m_propertyMacro, std::regex(ArgusCodeGeneratorUtil::s_propertyDelimiter), s_upropertyPrefix);
							outParsedFileContents[i].m_lines.push_back(overridePropertyText);
						}
					}
					else
					{
						outParsedFileContents[i].m_lines.push_back(s_propertyMacro);
					}

					if (!parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName.empty())
					{
						outParsedFileContents[i].m_lines.push_back(std::vformat("\tF{}Reference {}Reference;", std::make_format_args(parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName, parsedComponentData.m_componentVariableData[i][j].m_varName)));
					}
					else
					{
						const size_t exponentialDecaySmootherIndex = parsedComponentData.m_componentVariableData[i][j].m_typeName.find(s_exponentialDecaySmootherTypeName);
						if (exponentialDecaySmootherIndex != std::string::npos)
						{
							variable = "float";
							variable.append(" ");
							variable.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
							variable.append("DecayConstant");
							variable.append(" = 1.0f");
						}
						else
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
						variable.append(";");
						outParsedFileContents[i].m_lines.push_back(variable);
					}

					outParsedFileContents[i].m_lines.push_back("");
				}
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
					if (parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyIgnoreDelimiter) != std::string::npos)
					{
						continue;
					}

					if (parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyObservableDelimiter) != std::string::npos &&
						parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyObservablePropertyDeclarationDelimiter) == std::string::npos)
					{
						continue;
					}

					std::string variableAssignment = "\t";
					variableAssignment.append(parsedComponentData.m_componentNames[i]);
					variableAssignment.append("Ref->");
					variableAssignment.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
					variableAssignment.append(" = ");

					const size_t propertyStaticDataDelimiterIndex = parsedComponentData.m_componentVariableData[i][j].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter);
					const size_t exponentialDecaySmootherIndex = parsedComponentData.m_componentVariableData[i][j].m_typeName.find(s_exponentialDecaySmootherTypeName);
					if (propertyStaticDataDelimiterIndex != std::string::npos)
					{
						variableAssignment.append(std::vformat("{}Reference.GetId()", std::make_format_args(parsedComponentData.m_componentVariableData[i][j].m_varName)));
					}
					else if (exponentialDecaySmootherIndex != std::string::npos)
					{
						const size_t lengthTypeName = parsedComponentData.m_componentVariableData[i][j].m_typeName.length();
						variableAssignment.append(parsedComponentData.m_componentVariableData[i][j].m_typeName.substr(1, (lengthTypeName - 1)));
						variableAssignment.append("(");
						variableAssignment.append(parsedComponentData.m_componentVariableData[i][j].m_varName);
						variableAssignment.append("DecayConstant");
						variableAssignment.append(")");
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
		else if (cppLineText.find("%%%%%") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					if (parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName.empty())
					{
						continue;
					}

					outParsedFileContents[i].m_lines.push_back
					(
						std::vformat("\tArgusStaticData::AsyncPreLoadRecord<{}>({}Reference.GetId());", std::make_format_args(parsedComponentData.m_componentVariableData[i][j].m_staticDataTypeName, parsedComponentData.m_componentVariableData[i][j].m_varName))
					);
				}
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
