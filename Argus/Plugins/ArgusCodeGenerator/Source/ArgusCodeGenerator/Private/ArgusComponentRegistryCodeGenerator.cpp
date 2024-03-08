// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include "ArgusCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>

// Constants for file parsing
static constexpr std::string s_classDelimiter = "class";
static const size_t s_classDelimiterLength = s_classDelimiter.length();
static constexpr char s_inheritanceDelimiter = ':';
static constexpr char s_openBracketDelimiter = '{';

void ArgusComponentRegistryCodeGenerator::GenerateComponentRegistry()
{
	// Get a path to base project directory
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename =  FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	// Construct a file path to component definitions
	FString componentDefinitionsDirectory = *projectDirectory;
	componentDefinitionsDirectory.Append("Source/Argus/ECS/ComponentDefinitions");
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	// Iterate over ComponentDefinitions files.
	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Printing out found component definition paths from directory: %s"), ARGUS_FUNCNAME, *componentDefinitionsDirectory);
	std::vector<std::string> componentNames = std::vector<std::string>();
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		ParseComponentNamesFromFile(entry.path().string(), componentNames);
	}

	// TODO JAMES: Now that we are parsing the component names into a vector, we need to parse from template txt files to do string replacements.
	// TODO JAMES: After doing proper string replacements, we then need to write to the out files.
}

void ArgusComponentRegistryCodeGenerator::ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames)
{
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Faile to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
	}

	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Reading from file: %s"), ARGUS_FUNCNAME, *ueFilePath);

	std::string lineText;
	while (std::getline(inStream, lineText))
	{
		const size_t classDelimiterIndex = lineText.find(s_classDelimiter);
		if (classDelimiterIndex == std::string::npos)
		{
			continue;
		}
		const size_t postClassStartIndex = classDelimiterIndex + s_classDelimiterLength;

		const size_t inheritanceDelimiterIndex = lineText.find(s_inheritanceDelimiter);
		if (inheritanceDelimiterIndex != std::string::npos)
		{
			lineText = lineText.substr(postClassStartIndex, inheritanceDelimiterIndex - postClassStartIndex);
		}
		else
		{
			const size_t openBracketDelimiterIndex = lineText.find(s_openBracketDelimiter);
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
		outComponentNames.push_back(lineText);
	}
}
