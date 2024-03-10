// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include "ArgusCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>

// Constants for file parsing
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionSuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates/";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderTemplateFilename = "ArgusComponentRegistryHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppTemplateFilename = "ArgusComponentRegistryCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentHeaderTemplateFilename = "ComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateDefinitionsFilename = "ComponentCppTemplateDefinitions.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateFlushFilename = "ComponentCppTemplateFlush.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateResetFilename = "ComponentCppTemplateReset.txt";
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
	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Printing out found component definition paths from directory: %s"), ARGUS_FUNCNAME, *componentDefinitionsDirectory)
	ParseComponentRegistryHeaderTemplateParams params;
	params.inComponentNames = std::vector<std::string>();
	params.inIncludeStatements = std::vector<std::string>();
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		ParseComponentNamesFromFile(filePath, params.inComponentNames);
		ParseIncludeStatementsFromFile(filePath, params.inIncludeStatements);
	}

	// Construct a directory path to component templates
	FString templateDirectory = *projectDirectory;
	templateDirectory.Append(s_templateDirectorySuffix);
	FPaths::MakeStandardFilename(templateDirectory);
	const char* cStrTemplateDirectory = TCHAR_TO_UTF8(*templateDirectory);

	params.argusComponentRegistryHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentRegistryHeaderTemplateFilename);
	params.componentHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentHeaderTemplateFilename);

	std::vector<std::string> outParsedHeaderFileContents = std::vector<std::string>();
	ParseComponentRegistryHeaderTemplate(params, outParsedHeaderFileContents);

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
	std::ifstream inStream = std::ifstream(params.componentHeaderTemplateFilePath);
	const FString ueFilePath = FString(params.componentHeaderTemplateFilePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
	}

	std::vector<std::string> rawLines = std::vector<std::string>();
	std::string lineText;
	while (std::getline(inStream, lineText))
	{
		rawLines.push_back(lineText);
	}

	std::vector<std::string> parsedLines = std::vector<std::string>();
	for (std::string component : params.inComponentNames)
	{

	}

	std::ifstream inHeaderStream = std::ifstream(params.argusComponentRegistryHeaderTemplateFilePath);
	const FString ueHeaderFilePath = FString(params.argusComponentRegistryHeaderTemplateFilePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
	}

	std::string headerLineText;
	while (std::getline(inStream, headerLineText))
	{

	}
}
