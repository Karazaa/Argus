// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorUtil.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

DEFINE_LOG_CATEGORY(ArgusCodeGeneratorLog);

const char* ArgusCodeGeneratorUtil::s_propertyDelimiter = "ARGUS_PROPERTY";
const char* ArgusCodeGeneratorUtil::s_propertyIgnoreDelimiter = "ARGUS_IGNORE";
const char* ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter = "ARGUS_STATIC_DATA";
const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectorySuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_staticDataRecordDefinitionsDirectoryName = "RecordDefinitions";
const char* ArgusCodeGeneratorUtil::s_staticDataRecordDefinitionsDirectorySuffix = "Source/Argus/StaticData/RecordDefinitions";
const char* ArgusCodeGeneratorUtil::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates/";
const char* ArgusCodeGeneratorUtil::s_structDelimiter = "struct";
const char* ArgusCodeGeneratorUtil::s_varDelimiter = "m_";

FString ArgusCodeGeneratorUtil::GetProjectDirectory()
{
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename = FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	return projectDirectory;
}

FString ArgusCodeGeneratorUtil::GetTemplateDirectory(const char* subdirectorySuffix)
{
	FString templateDirectory = *GetProjectDirectory();
	templateDirectory.Append(s_templateDirectorySuffix);
	templateDirectory.Append(subdirectorySuffix);
	FPaths::MakeStandardFilename(templateDirectory);

	return templateDirectory;
}

FString ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory()
{
	FString componentDefinitionsDirectory = *GetProjectDirectory();
	componentDefinitionsDirectory.Append(s_componentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);

	return componentDefinitionsDirectory;
}

FString ArgusCodeGeneratorUtil::GetStaticDataRecordDefinitionsDirectory()
{
	FString recordDefinitionsDirectory = *GetProjectDirectory();
	recordDefinitionsDirectory.Append(s_staticDataRecordDefinitionsDirectorySuffix);
	FPaths::MakeStandardFilename(recordDefinitionsDirectory);

	return recordDefinitionsDirectory;
}

bool ArgusCodeGeneratorUtil::ParseComponentData(ParseComponentDataOutput& output)
{
	bool didSucceed = true;

	FString componentDefinitionsDirectory = ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory();
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		didSucceed &= ParseComponentDataFromFile(filePath, output);
	}

	return didSucceed;
}

bool ArgusCodeGeneratorUtil::ParseComponentDataFromFile(const std::string& filePath, ParseComponentDataOutput& output)
{
	const size_t componentDefinitionIndex = filePath.find(s_componentDefinitionDirectoryName);
	std::string includeStatement = "#include \"";
	std::string dataAssetIncludeStatement = "#include \"..\\";
	std::string componentPath = filePath.substr(componentDefinitionIndex);
	includeStatement.append(componentPath);
	includeStatement.append("\"");
	dataAssetIncludeStatement.append(componentPath);
	dataAssetIncludeStatement.append("\"");
	output.m_componentRegistryIncludeStatements.push_back(includeStatement);

	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Reading from file: %s"), ARGUS_FUNCNAME, *ueFilePath);

	std::string lineText;
	bool didParsePropertyDeclaration = false;
	while (std::getline(inStream, lineText))
	{
		if (ParseStructDeclarations(lineText, dataAssetIncludeStatement, output))
		{
			continue;
		}

		if (ParseVariableDeclarations(lineText, didParsePropertyDeclaration, output))
		{
			didParsePropertyDeclaration = false;
			continue;
		}

		if (ParsePropertyMacro(lineText, output))
		{
			didParsePropertyDeclaration = true;
			continue;
		}
	}
	inStream.close();
	return true;
}

bool ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents)
{
	// Read from definitions template
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	std::vector<std::string> rawLines = std::vector<std::string>();
	std::string lineText;
	while (std::getline(inStream, lineText))
	{
		rawLines.push_back(lineText);
	}
	inStream.close();

	// Parse per component template into one section
	for (std::string component : componentNames)
	{
		for (std::string rawLine : rawLines)
		{
			outFileContents.push_back(std::regex_replace(rawLine, std::regex("#####"), component));
		}
	}
	return true;
}

bool ArgusCodeGeneratorUtil::ParseStaticDataRecords(ParseStaticDataRecordsOutput& output)
{
	bool didSucceed = true;

	FString recordDefinitionsDirectory = ArgusCodeGeneratorUtil::GetStaticDataRecordDefinitionsDirectory();
	const std::string finalizedRecordDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*recordDefinitionsDirectory));

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedRecordDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
	}

	return didSucceed;
}

bool ArgusCodeGeneratorUtil::WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents)
{
	std::ofstream outStream = std::ofstream(filePath, std::ofstream::out | std::ofstream::trunc);
	const FString ueFilePath = FString(filePath.c_str());
	if (!outStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to write to output file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	for (std::string line : inFileContents)
	{
		outStream << line << std::endl;
	}
	outStream.close();
	return true;
}

bool ArgusCodeGeneratorUtil::ParseStructDeclarations(std::string lineText, const std::string& componentDataAssetIncludeStatement, ParseComponentDataOutput& output)
{
	const size_t structDelimiterLength = std::strlen(s_structDelimiter);
	const size_t classDelimiterIndex = lineText.find(s_structDelimiter);
	if (classDelimiterIndex == std::string::npos)
	{
		return false;
	}

	const size_t postClassStartIndex = classDelimiterIndex + structDelimiterLength;

	const size_t inheritanceDelimiterIndex = lineText.find(':');
	if (inheritanceDelimiterIndex != std::string::npos)
	{
		lineText = lineText.substr(postClassStartIndex, inheritanceDelimiterIndex - postClassStartIndex);
	}
	else
	{
		const size_t openBracketDelimiterIndex = lineText.find('{');
		if (openBracketDelimiterIndex != std::string::npos)
		{
			lineText = lineText.substr(postClassStartIndex, openBracketDelimiterIndex - postClassStartIndex);
		}
		else
		{
			lineText = lineText.substr(postClassStartIndex, lineText.length() - 1);
		}
	}

	std::erase(lineText, ' ');
	output.m_componentNames.push_back(lineText);
	output.m_componentDataAssetIncludeStatements.push_back(componentDataAssetIncludeStatement);
	output.m_componentVariableData.push_back(std::vector<ComponentVariableData>());
	return true;
}

bool ArgusCodeGeneratorUtil::ParsePropertyMacro(std::string lineText, ParseComponentDataOutput& output)
{
	const size_t propertyDelimiterIndex = lineText.find(s_propertyDelimiter);
	const size_t propertyIgnoreDelimiterIndex = lineText.find(s_propertyIgnoreDelimiter);
	const size_t propertyStaticDataDelimiterIndex = lineText.find(s_propertyStaticDataDelimiter);
	if (propertyDelimiterIndex == std::string::npos && 
		propertyIgnoreDelimiterIndex == std::string::npos &&
		propertyStaticDataDelimiterIndex == std::string::npos)
	{
		return false;
	}

	ComponentVariableData variableData;
	variableData.m_propertyMacro = lineText;
	output.m_componentVariableData.back().push_back(variableData);

	return true;
}

bool ArgusCodeGeneratorUtil::ParseVariableDeclarations(std::string lineText, bool withProperty, ParseComponentDataOutput& output)
{
	const size_t variableDelimiterIndex = lineText.find(s_varDelimiter);
	if (variableDelimiterIndex == std::string::npos)
	{
		return false;
	}
	ComponentVariableData variableData;
	variableData.m_typeName = lineText.substr(0, variableDelimiterIndex);

	const size_t equalDelimiterIndex = lineText.find('=');
	const size_t endDelimiterIndex = lineText.find(';');
	if (equalDelimiterIndex != std::string::npos)
	{
		variableData.m_varName = lineText.substr(variableDelimiterIndex, equalDelimiterIndex - variableDelimiterIndex);
		if (endDelimiterIndex != std::string::npos)
		{
			variableData.m_defaultValue = lineText.substr(equalDelimiterIndex + 1, endDelimiterIndex - (equalDelimiterIndex + 1));
		}
		else
		{
			variableData.m_defaultValue = "";
		}
	}
	else if (endDelimiterIndex != std::string::npos)
	{
		variableData.m_varName = lineText.substr(variableDelimiterIndex, endDelimiterIndex - variableDelimiterIndex);
		variableData.m_defaultValue = "";
	}
	else
	{
		return false;
	}

	std::erase(variableData.m_typeName, ' ');
	std::erase(variableData.m_varName, ' ');
	std::erase(variableData.m_defaultValue, ' ');

	const int componentVariableCount = output.m_componentVariableData.back().size();
	if (withProperty && componentVariableCount)
	{
		const int index = componentVariableCount - 1;
		output.m_componentVariableData.back()[index].m_typeName = variableData.m_typeName;
		output.m_componentVariableData.back()[index].m_varName = variableData.m_varName;
		output.m_componentVariableData.back()[index].m_defaultValue = variableData.m_defaultValue;
	}
	else
	{
		for (int i = 0; i < output.m_componentVariableData.back().size(); ++i)
		{
			const size_t variableNameIndex = variableData.m_varName.find(output.m_componentVariableData.back()[i].m_varName);
			if (variableNameIndex != std::string::npos)
			{
				return false;
			}
		}

		output.m_componentVariableData.back().push_back(variableData);
	}
	return true;
}
