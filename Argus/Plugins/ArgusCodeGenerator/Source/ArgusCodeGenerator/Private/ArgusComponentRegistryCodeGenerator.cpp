// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include "ArgusCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

// Constants for file parsing
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_componentDefinitionDirectorySuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusComponentRegistryCodeGenerator::s_componentRegistryDirectorySuffix = "Source/Argus/ECS/";
const char* ArgusComponentRegistryCodeGenerator::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates/";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderTemplateFilename = "ArgusComponentRegistryHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppTemplateFilename = "ArgusComponentRegistryCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentHeaderTemplateFilename = "ComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateDefinitionsFilename = "ComponentCppTemplateDefinitions.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateFlushFilename = "ComponentCppTemplateFlush.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateResetFilename = "ComponentCppTemplateReset.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderFilename = "ArgusComponentRegistry.h";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppFilename = "ArgusComponentRegistry.cpp";
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
	componentDefinitionsDirectory.Append(s_componentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	// Iterate over ComponentDefinitions files.
	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Printing out found component definition paths from directory: %s"), ARGUS_FUNCNAME, *componentDefinitionsDirectory)
	ParseComponentRegistryTemplateParams params;
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
	params.argusComponentRegistryCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentRegistryCppTemplateFilename);
	params.componentHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentHeaderTemplateFilename);
	params.componentCppTemplateDefinitionsFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateDefinitionsFilename);
	params.componentCppTemplateResetFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateResetFilename);
	params.componentCppTemplateFlushFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateFlushFilename);

	// Parse header file
	std::vector<std::string> outParsedHeaderFileContents = std::vector<std::string>();
	ParseComponentRegistryHeaderTemplate(params, outParsedHeaderFileContents);

	// Parse cpp file
	std::vector<std::string> outParsedCppFileContents = std::vector<std::string>();
	ParseComponentRegistryCppTemplate(params, outParsedCppFileContents);

	// Construct a directory path to component templates
	FString registryDirectory = *projectDirectory;
	registryDirectory.Append(s_componentRegistryDirectorySuffix);
	FPaths::MakeStandardFilename(registryDirectory);
	const char* cStrRegistryDirectory = TCHAR_TO_UTF8(*registryDirectory);

	// Write out header and cpp file
	WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryHeaderFilename), outParsedHeaderFileContents);
	WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryCppFilename), outParsedCppFileContents);
}

void ArgusComponentRegistryCodeGenerator::ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames)
{
	const size_t classDelimiterLength = std::strlen(s_classDelimiter);
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
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
	inStream.close();
}

void ArgusComponentRegistryCodeGenerator::ParseIncludeStatementsFromFile(const std::string& filePath, std::vector<std::string>& outIncludeStatements)
{
	const size_t componentDefinitionIndex = filePath.find(s_componentDefinitionDirectoryName);
	std::string includeStatement = "#include \"";
	includeStatement.append(filePath.substr(componentDefinitionIndex));
	includeStatement.append("\"");
	outIncludeStatements.push_back(includeStatement);
}

void ArgusComponentRegistryCodeGenerator::ParseComponentRegistryHeaderTemplate(const ParseComponentRegistryTemplateParams& params, std::vector<std::string>& outFileContents)
{
	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

	std::ifstream inHeaderStream = std::ifstream(params.argusComponentRegistryHeaderTemplateFilePath);
	const FString ueHeaderFilePath = FString(params.argusComponentRegistryHeaderTemplateFilePath.c_str());
	if (!inHeaderStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueHeaderFilePath);
		return;
	}

	std::string headerLineText;
	while (std::getline(inHeaderStream, headerLineText))
	{
		if (headerLineText.find("@@@@@") != std::string::npos)
		{
			for (std::string includeStatement : params.inIncludeStatements)
			{
				outFileContents.push_back(includeStatement);
			}
		}
		else if (headerLineText.find("#####") != std::string::npos)
		{
			for (std::string parsedLine : parsedLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else
		{
			outFileContents.push_back(headerLineText);
		}
	}
	inHeaderStream.close();
}

void ArgusComponentRegistryCodeGenerator::ParseComponentRegistryCppTemplate(const ParseComponentRegistryTemplateParams& params, std::vector<std::string>& outFileContents)
{
	// Parse definitions template into one section
	std::vector<std::string> parsedDefinitionLines = std::vector<std::string>();
	ParseComponentSpecificTemplate(params.componentCppTemplateDefinitionsFilePath, params.inComponentNames, parsedDefinitionLines);

	// Parse reset template into one section
	std::vector<std::string> parsedResetLines = std::vector<std::string>();
	ParseComponentSpecificTemplate(params.componentCppTemplateResetFilePath, params.inComponentNames, parsedResetLines);

	// Parse reset template into one section
	std::vector<std::string> parsedFlushLines = std::vector<std::string>();
	ParseComponentSpecificTemplate(params.componentCppTemplateFlushFilePath, params.inComponentNames, parsedFlushLines);

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

	std::ifstream inCppStream = std::ifstream(params.argusComponentRegistryCppTemplateFilePath);
	const FString ueCppFilePath = FString(params.argusComponentRegistryCppTemplateFilePath.c_str());
	if (!inCppStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueCppFilePath);
		return;
	}

	std::string cppLineText;
	while (std::getline(inCppStream, cppLineText))
	{
		if (cppLineText.find("@@@@@") != std::string::npos)
		{
			for (std::string parsedLine : parsedDefinitionLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("#####") != std::string::npos)
		{
			for (std::string parsedLine : parsedResetLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("$$$$$") != std::string::npos)
		{
			for (std::string parsedLine : parsedFlushLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else
		{
			outFileContents.push_back(cppLineText);
		}
	}
	inCppStream.close();
}

void ArgusComponentRegistryCodeGenerator::ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents)
{
	// Read from definitions template
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
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
}

void ArgusComponentRegistryCodeGenerator::WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents)
{
	std::ofstream outStream = std::ofstream(filePath, std::ofstream::out | std::ofstream::trunc);
	const FString ueFilePath = FString(filePath.c_str());
	if (!outStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to write to output file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return;
	}

	for (std::string line : inFileContents)
	{
		outStream << line << std::endl;
	}
	outStream.close();
}