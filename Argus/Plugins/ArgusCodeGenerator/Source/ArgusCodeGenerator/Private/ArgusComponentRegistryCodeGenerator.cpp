// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include "ArgusCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>

// Constants for file parsing
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionSuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderTemplateFilename = "ArgusComponentRegistryHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppTemplateFilename = "ArgusComponentRegistryCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentHeaderTemplateFilename = "ComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateFilename = "ComponentCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_classDelimiter = "class";
const char  ArgusComponentRegistryCodeGenerator::s_inheritanceDelimiter = ':';
const char  ArgusComponentRegistryCodeGenerator::s_openBracketDelimiter = '{';

void ArgusComponentRegistryCodeGenerator::GenerateComponentRegistry()
{
	// Get a path to base project directory
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename =  FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	// Construct a file path to component definitions
	FString componentDefinitionsDirectory = *projectDirectory;
	componentDefinitionsDirectory.Append(s_componentDefinitionSuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	// Iterate over ComponentDefinitions files.
	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Printing out found component definition paths from directory: %s"), ARGUS_FUNCNAME, *componentDefinitionsDirectory);
	std::vector<std::string> componentNames = std::vector<std::string>();
	std::vector<std::string> includeStatements = std::vector<std::string>();
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		ParseComponentNamesFromFile(filePath, componentNames);
		ParseIncludeStatementsFromFile(filePath, includeStatements);
	}

	// TODO JAMES: Now that we are parsing the component names into a vector, we need to parse from template .txt files to do string replacements.
	// TODO JAMES: After doing proper string replacements, we then need to write to the out files.
}

void ArgusComponentRegistryCodeGenerator::ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames)
{
	const size_t classDelimiterLength = std::strlen(s_classDelimiter);
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
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
		const size_t postClassStartIndex = classDelimiterIndex + classDelimiterLength;

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

void ArgusComponentRegistryCodeGenerator::ParseIncludeStatementsFromFile(const std::string& filePath, std::vector<std::string>& outIncludeStatements)
{
	const size_t componentDefinitionIndex = filePath.find(s_componentDefinitionDirectoryName);
	std::string includeStatement = "#include \"";
	includeStatement.append(filePath.substr(componentDefinitionIndex));
	includeStatement.append("\"");
	outIncludeStatements.push_back(includeStatement);
}

void ArgusComponentRegistryCodeGenerator::ParseComponentRegistryHeaderTemplate(const ParseComponentRegistryHeaderTemplateParams& params, std::vector<std::string>& outFileContents)
{
	std::ifstream inStream = std::ifstream(params.argusComponentRegistryHeaderTemplateFilePath);
	const FString ueFilePath = FString(params.argusComponentRegistryHeaderTemplateFilePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
	}
}
