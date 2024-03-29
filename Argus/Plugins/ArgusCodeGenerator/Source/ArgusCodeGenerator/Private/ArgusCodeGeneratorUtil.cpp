// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorUtil.h"
#include <filesystem>
#include <fstream>
#include <regex>

DEFINE_LOG_CATEGORY(ArgusCodeGeneratorLog);

const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectorySuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_structDelimiter = "struct";

FString ArgusCodeGeneratorUtil::GetProjectDirectory()
{
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename = FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	return projectDirectory;
}

FString ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory()
{
	FString componentDefinitionsDirectory = *GetProjectDirectory();
	componentDefinitionsDirectory.Append(s_componentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);

	return componentDefinitionsDirectory;
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
	includeStatement.append(filePath.substr(componentDefinitionIndex));
	includeStatement.append("\"");
	output.m_componentRegistryIncludeStatements.push_back(includeStatement);

	const size_t structDelimiterLength = std::strlen(s_structDelimiter);
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Reading from file: %s"), ARGUS_FUNCNAME, *ueFilePath);

	std::string lineText;
	while (std::getline(inStream, lineText))
	{
		const size_t classDelimiterIndex = lineText.find(s_structDelimiter);
		if (classDelimiterIndex == std::string::npos)
		{
			continue;
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
