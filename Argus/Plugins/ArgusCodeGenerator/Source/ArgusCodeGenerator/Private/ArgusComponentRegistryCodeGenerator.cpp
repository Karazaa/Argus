// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include <filesystem>
#include <fstream>
#include <regex>

// Constants for file parsing
const char* ArgusComponentRegistryCodeGenerator::s_componentRegistryDirectorySuffix = "Source/Argus/ECS/";
const char* ArgusComponentRegistryCodeGenerator::s_ecsTestsDirectorySuffix = "Tests/";
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

void ArgusComponentRegistryCodeGenerator::GenerateComponentRegistry(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS Component code."), ARGUS_FUNCNAME)

	FString projectDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();

	bool didSucceed = true;

	// Iterate over ComponentDefinitions files.
	ParseComponentTemplateParams params;
	params.inComponentNames = parsedComponentData.m_componentNames;
	params.inIncludeStatements = parsedComponentData.m_componentRegistryIncludeStatements;

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

	// Construct a directory path to registry location and to tests location. 
	FString registryDirectory = *projectDirectory;
	registryDirectory.Append(s_componentRegistryDirectorySuffix);
	FString testsDirectory = *registryDirectory;
	testsDirectory.Append(s_ecsTestsDirectorySuffix);
	FPaths::MakeStandardFilename(registryDirectory);
	FPaths::MakeStandardFilename(testsDirectory);
	const char* cStrRegistryDirectory = TCHAR_TO_UTF8(*registryDirectory);
	const char* cStrTestsDirectory = TCHAR_TO_UTF8(*testsDirectory);

	// Write out header, cpp, and tests file
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryHeaderFilename), outParsedHeaderFileContents);
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryCppFilename), outParsedCppFileContents);
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrTestsDirectory).append(s_argusComponentSizeTestsFilename), outParsedTestsFileContents);
	
	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS Component implementations."), ARGUS_FUNCNAME)
	}
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentRegistryHeaderTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

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
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateDefinitionsFilePath, params.inComponentNames, parsedDefinitionLines);

	// Parse reset template into one section
	std::vector<std::string> parsedResetLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateResetFilePath, params.inComponentNames, parsedResetLines);

	// Parse reset template into one section
	std::vector<std::string> parsedFlushLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateFlushFilePath, params.inComponentNames, parsedFlushLines);

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

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
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.perComponentSizeTestsTemplateFilePath, params.inComponentNames, parsedLines);

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