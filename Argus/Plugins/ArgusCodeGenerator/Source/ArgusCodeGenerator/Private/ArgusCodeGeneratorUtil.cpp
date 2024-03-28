// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorUtil.h"
#include <fstream>
#include <regex>

DEFINE_LOG_CATEGORY(ArgusCodeGeneratorLog);

const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectorySuffix = "Source/Argus/ECS/ComponentDefinitions";

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
