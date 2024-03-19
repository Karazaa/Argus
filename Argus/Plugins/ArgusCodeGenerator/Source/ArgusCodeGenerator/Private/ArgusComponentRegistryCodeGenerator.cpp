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
const char* ArgusComponentRegistryCodeGenerator::s_ecsTestsDirectorySuffix = "Source/Argus/ECS/Tests";
const char* ArgusComponentRegistryCodeGenerator::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates/";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderTemplateFilename = "ArgusComponentRegistryHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppTemplateFilename = "ArgusComponentRegistryCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentHeaderTemplateFilename = "ComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateDefinitionsFilename = "ComponentCppTemplateDefinitions.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateFlushFilename = "ComponentCppTemplateFlush.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateResetFilename = "ComponentCppTemplateReset.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentSizeTestsTemplateFilename = "ArgusComponentSizeTestsTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_perComponentSizeTestsTemplateFilename = "PerComponentSizeTestsTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderFilename = "ArgusComponentRegistry.h";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppFilename = "ArgusComponentRegistry.cpp";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentSizeTestsFilename = "ArgusComponentSizeTests.cpp";
const char* ArgusComponentRegistryCodeGenerator::s_structDelimiter = "struct";

void ArgusComponentRegistryCodeGenerator::GenerateComponentRegistry()
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS Component code."), ARGUS_FUNCNAME)

	// Get a path to base project directory
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename =  FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	// Construct a file path to component definitions
	FString componentDefinitionsDirectory = *projectDirectory;
	componentDefinitionsDirectory.Append(s_componentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	bool didSucceed = true;

	// Iterate over ComponentDefinitions files.
	ParseComponentTemplateParams params;
	params.inComponentNames = std::vector<std::string>();
	params.inIncludeStatements = std::vector<std::string>();
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		didSucceed &= ParseComponentNamesFromFile(filePath, params.inComponentNames);
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
	params.argusComponentSizeTestsTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentSizeTestsTemplateFilename);
	params.perComponentSizeTestsTemplateFilePath = std::string(cStrTemplateDirectory).append(s_perComponentSizeTestsTemplateFilename);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementations."), ARGUS_FUNCNAME)
	// Parse header file
	std::vector<std::string> outParsedHeaderFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentRegistryHeaderTemplate(params, outParsedHeaderFileContents);

	// Parse cpp file
	std::vector<std::string> outParsedCppFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentRegistryCppTemplate(params, outParsedCppFileContents);

	// Parse tests file
	std::vector<std::string> outParsedTestsFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentSizeTestsTemplate(params, outParsedTestsFileContents);

	// Construct a directory path to component templates
	FString registryDirectory = *projectDirectory;
	registryDirectory.Append(s_componentRegistryDirectorySuffix);
	FPaths::MakeStandardFilename(registryDirectory);
	const char* cStrRegistryDirectory = TCHAR_TO_UTF8(*registryDirectory);

	// Write out header and cpp file
	didSucceed &= WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryHeaderFilename), outParsedHeaderFileContents);
	didSucceed &= WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryCppFilename), outParsedCppFileContents);
	
	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS Component implementations."), ARGUS_FUNCNAME)
	}
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames)
{
	const size_t classDelimiterLength = std::strlen(s_structDelimiter);
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
		const size_t postClassStartIndex = classDelimiterIndex + classDelimiterLength;

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
		outComponentNames.push_back(lineText);
	}
	inStream.close();
	return true;
}

void ArgusComponentRegistryCodeGenerator::ParseIncludeStatementsFromFile(const std::string& filePath, std::vector<std::string>& outIncludeStatements)
{
	const size_t componentDefinitionIndex = filePath.find(s_componentDefinitionDirectoryName);
	std::string includeStatement = "#include \"";
	includeStatement.append(filePath.substr(componentDefinitionIndex));
	includeStatement.append("\"");
	outIncludeStatements.push_back(includeStatement);
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentRegistryHeaderTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

	std::ifstream inHeaderStream = std::ifstream(params.argusComponentRegistryHeaderTemplateFilePath);
	const FString ueHeaderFilePath = FString(params.argusComponentRegistryHeaderTemplateFilePath.c_str());
	if (!inHeaderStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueHeaderFilePath);
		return false;
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
		else if (headerLineText.find("%%%%%") != std::string::npos)
		{
			outFileContents.push_back(std::regex_replace(headerLineText, std::regex("%%%%%"), std::to_string(params.inComponentNames.size())));
		}
		else
		{
			outFileContents.push_back(headerLineText);
		}
	}
	inHeaderStream.close();
	return didSucceed;
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentRegistryCppTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse definitions template into one section
	std::vector<std::string> parsedDefinitionLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.componentCppTemplateDefinitionsFilePath, params.inComponentNames, parsedDefinitionLines);

	// Parse reset template into one section
	std::vector<std::string> parsedResetLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.componentCppTemplateResetFilePath, params.inComponentNames, parsedResetLines);

	// Parse reset template into one section
	std::vector<std::string> parsedFlushLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.componentCppTemplateFlushFilePath, params.inComponentNames, parsedFlushLines);

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

	std::ifstream inCppStream = std::ifstream(params.argusComponentRegistryCppTemplateFilePath);
	const FString ueCppFilePath = FString(params.argusComponentRegistryCppTemplateFilePath.c_str());
	if (!inCppStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueCppFilePath);
		return false;
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
	return didSucceed;
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentSizeTestsTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ParseComponentSpecificTemplate(params.perComponentSizeTestsTemplateFilePath, params.inComponentNames, parsedLines);

	std::ifstream inTestsStream = std::ifstream(params.argusComponentSizeTestsTemplateFilePath);
	const FString ueTestsFilePath = FString(params.argusComponentSizeTestsTemplateFilePath.c_str());
	if (!inTestsStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTestsFilePath);
		return false;
	}

	std::string testsLineText;
	while (std::getline(inTestsStream, testsLineText))
	{
		if (testsLineText.find("#####") != std::string::npos)
		{
			for (std::string parsedLine : parsedLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else
		{
			outFileContents.push_back(testsLineText);
		}
	}
	inTestsStream.close();
	return didSucceed;
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents)
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

bool ArgusComponentRegistryCodeGenerator::WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents)
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