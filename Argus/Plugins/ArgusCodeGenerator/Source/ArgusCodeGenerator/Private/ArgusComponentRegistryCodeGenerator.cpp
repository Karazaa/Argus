// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistryCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

// Constants for file parsing
const char* ArgusComponentRegistryCodeGenerator::s_componentRegistryDirectorySuffix = "Source/Argus/ECS/";
const char* ArgusComponentRegistryCodeGenerator::s_ecsTestsDirectorySuffix = "Tests/";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderTemplateFilename = "ArgusComponentRegistryHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppTemplateFilename = "ArgusComponentRegistryCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentHeaderTemplateFilename = "ComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_dynamicAllocComponentHeaderTemplateFilename = "DynamicAllocComponentHeaderTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateDefinitionsFilename = "ComponentCppTemplateDefinitions.txt";
const char* ArgusComponentRegistryCodeGenerator::s_dynamicAllocComponentCppTemplateDefinitionsFilename = "DynamicAllocComponentCppTemplateDefinitions.txt";
const char* ArgusComponentRegistryCodeGenerator::s_removeComponentCppTemplateBitsetFilename = "RemoveComponentCppTemplateBitset.txt";
const char* ArgusComponentRegistryCodeGenerator::s_removeComponentCppTemplateValuesFilename = "RemoveComponentCppTemplateValues.txt";
const char* ArgusComponentRegistryCodeGenerator::s_removeComponentCppTemplateDynamicAllocFilename = "RemoveComponentCppTemplateDynamicAlloc.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateFlushFilename = "ComponentCppTemplateFlush.txt";
const char* ArgusComponentRegistryCodeGenerator::s_componentCppTemplateResetFilename = "ComponentCppTemplateReset.txt";
const char* ArgusComponentRegistryCodeGenerator::s_dynamicAllocComponentCppTemplateResetFilename = "DynamicAllocComponentCppTemplateReset.txt";
const char* ArgusComponentRegistryCodeGenerator::s_debugStringLinesTemplateFilename = "AppendDebugStringCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_debugDrawLinesTemplateFilename = "DebugDrawCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_owningEntityIdLinesTemplateFilename = "OwningEntityIdCppTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentSizeTestsTemplateFilename = "ArgusComponentSizeTestsTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_perComponentSizeTestsTemplateFilename = "PerComponentSizeTestsTemplate.txt";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryHeaderFilename = "ArgusComponentRegistry.h";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentRegistryCppFilename = "ArgusComponentRegistry.cpp";
const char* ArgusComponentRegistryCodeGenerator::s_argusComponentSizeTestsFilename = "ArgusComponentSizeTests.cpp";
const char* ArgusComponentRegistryCodeGenerator::s_componentRegistryTemplateDirectorySuffix = "ComponentRegistry/";

void ArgusComponentRegistryCodeGenerator::GenerateComponentRegistryCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS component code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Iterate over ComponentDefinitions files.
	ParseComponentTemplateParams params;
	params.inComponentNames = parsedComponentData.m_componentNames;
	params.inDynamicAllocComponentNames = parsedComponentData.m_dynamicAllocComponentNames;
	params.inIncludeStatements = parsedComponentData.m_componentRegistryIncludeStatements;
	params.inDynamicAllocIncludeStatements = parsedComponentData.m_dynamicAllocComponentRegistryIncludeStatements;

	// Construct a directory path to component registry templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentRegistryTemplateDirectorySuffix));

	params.argusComponentRegistryHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentRegistryHeaderTemplateFilename);
	params.argusComponentRegistryCppTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentRegistryCppTemplateFilename);
	params.componentHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_componentHeaderTemplateFilename);
	params.dynamicAllocComponentHeaderTemplateFilePath = std::string(cStrTemplateDirectory).append(s_dynamicAllocComponentHeaderTemplateFilename);
	params.componentCppTemplateDefinitionsFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateDefinitionsFilename);
	params.dynamicAllocComponentCppTemplateDefinitionsFilePath = std::string(cStrTemplateDirectory).append(s_dynamicAllocComponentCppTemplateDefinitionsFilename);
	params.removeComponentCppTemplateBitsetFilePath = std::string(cStrTemplateDirectory).append(s_removeComponentCppTemplateBitsetFilename);
	params.removeComponentCppTemplateValuesFilePath = std::string(cStrTemplateDirectory).append(s_removeComponentCppTemplateValuesFilename);
	params.removeComponentCppTemplateDynamicAllocFilePath = std::string(cStrTemplateDirectory).append(s_removeComponentCppTemplateDynamicAllocFilename);
	params.componentCppTemplateResetFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateResetFilename);
	params.componentCppTemplateFlushFilePath = std::string(cStrTemplateDirectory).append(s_componentCppTemplateFlushFilename);
	params.dynamicAllocComponentCppTemplateResetFilePath = std::string(cStrTemplateDirectory).append(s_dynamicAllocComponentCppTemplateResetFilename);
	params.debugStringLinesTemplateFilePath = std::string(cStrTemplateDirectory).append(s_debugStringLinesTemplateFilename);
	params.debugDrawLinesTemplateFilePath = std::string(cStrTemplateDirectory).append(s_debugDrawLinesTemplateFilename);
	params.owningEntityIdLinesTemplateFilePath = std::string(cStrTemplateDirectory).append(s_owningEntityIdLinesTemplateFilename);
	params.argusComponentSizeTestsTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusComponentSizeTestsTemplateFilename);
	params.perComponentSizeTestsTemplateFilePath = std::string(cStrTemplateDirectory).append(s_perComponentSizeTestsTemplateFilename);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementations."), ARGUS_FUNCNAME)
	// Parse header file
	std::vector<std::string> outParsedHeaderFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentRegistryHeaderTemplateWithReplacements(params, outParsedHeaderFileContents);

	// Parse cpp file
	std::vector<std::string> outParsedCppFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentRegistryCppTemplateWithReplacements(params, outParsedCppFileContents);

	// Parse tests file
	std::vector<std::string> outParsedTestsFileContents = std::vector<std::string>();
	didSucceed &= ParseComponentSizeTestsTemplateWithReplacements(params, outParsedTestsFileContents);

	// Construct a directory path to registry location and to tests location. 
	FString registryDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	registryDirectory.Append(s_componentRegistryDirectorySuffix);
	FString testsDirectory = *registryDirectory;
	testsDirectory.Append(s_ecsTestsDirectorySuffix);
	FPaths::MakeStandardFilename(registryDirectory);
	FPaths::MakeStandardFilename(testsDirectory);
	const char* cStrRegistryDirectory = ARGUS_FSTRING_TO_CHAR(registryDirectory);
	const char* cStrTestsDirectory = ARGUS_FSTRING_TO_CHAR(testsDirectory);

	// Write out header, cpp, and tests file
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryHeaderFilename), outParsedHeaderFileContents);
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrRegistryDirectory).append(s_argusComponentRegistryCppFilename), outParsedCppFileContents);
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrTestsDirectory).append(s_argusComponentSizeTestsFilename), outParsedTestsFileContents);
	
	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS component implementations."), ARGUS_FUNCNAME)
	}
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentRegistryHeaderTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse per component template into one section
	std::vector<std::string> parsedLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentHeaderTemplateFilePath, params.inComponentNames, parsedLines);

	// Parse per dynamic alloc component template into one section
	std::vector<std::string> parsedDynamicAllocLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.dynamicAllocComponentHeaderTemplateFilePath, params.inDynamicAllocComponentNames, parsedDynamicAllocLines);

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
		else if (headerLineText.find("^^^^^") != std::string::npos)
		{
			for (std::string includeStatement : params.inDynamicAllocIncludeStatements)
			{
				outFileContents.push_back(includeStatement);
			}
		}
		else if (headerLineText.find("%%%%%") != std::string::npos)
		{
			outFileContents.push_back(std::regex_replace(headerLineText, std::regex("%%%%%"), std::to_string(params.inComponentNames.size() + params.inDynamicAllocComponentNames.size())));
		}
		else if (headerLineText.find("#####") != std::string::npos)
		{
			for (std::string parsedLine : parsedLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (headerLineText.find("&&&&&") != std::string::npos)
		{
			for (std::string parsedLine : parsedDynamicAllocLines)
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
	return didSucceed;
}

bool ArgusComponentRegistryCodeGenerator::ParseComponentRegistryCppTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
{
	bool didSucceed = true;

	// Parse definitions template into one section
	std::vector<std::string> parsedDefinitionLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateDefinitionsFilePath, params.inComponentNames, parsedDefinitionLines);

	// Parse dynamic alloc definitions template into one section
	std::vector<std::string> parsedDynamicAllocDefinitionLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.dynamicAllocComponentCppTemplateDefinitionsFilePath, params.inDynamicAllocComponentNames, parsedDynamicAllocDefinitionLines);

	// Parse dynamic alloc definitions template into one section
	std::vector<std::string> parsedRemoveComponentBitset = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.removeComponentCppTemplateBitsetFilePath, params.inComponentNames, parsedRemoveComponentBitset);

	// Parse dynamic alloc definitions template into one section
	std::vector<std::string> parsedRemoveComponentValues = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.removeComponentCppTemplateValuesFilePath, params.inComponentNames, parsedRemoveComponentValues);

	// Parse dynamic alloc definitions template into one section
	std::vector<std::string> parsedRemoveComponentDynamicAlloc = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.removeComponentCppTemplateDynamicAllocFilePath, params.inDynamicAllocComponentNames, parsedRemoveComponentDynamicAlloc);

	// Parse reset template into one section
	std::vector<std::string> parsedResetLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateResetFilePath, params.inComponentNames, parsedResetLines);

	// Parse flush template into one section
	std::vector<std::string> parsedFlushLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.componentCppTemplateFlushFilePath, params.inComponentNames, parsedFlushLines);

	// Parse reset template into one section
	std::vector<std::string> parsedDynamicAllocResetLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.dynamicAllocComponentCppTemplateResetFilePath, params.inDynamicAllocComponentNames, parsedDynamicAllocResetLines);

	// Parse owning entity id logic into one section
	std::vector<std::string> parsedOwningEntityIdLines = std::vector<std::string>();
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.owningEntityIdLinesTemplateFilePath, params.inComponentNames, parsedOwningEntityIdLines);

	// Parse debug string lines template into one section
	std::vector<std::string> parsedDebugStringLines = std::vector<std::string>();
	std::vector<std::string> parsedDebugDrawLines = std::vector<std::string>();
	std::vector<std::string> allComponentNames = std::vector<std::string>();
	allComponentNames.insert(allComponentNames.end(), params.inComponentNames.begin(), params.inComponentNames.end());
	allComponentNames.insert(allComponentNames.end(), params.inDynamicAllocComponentNames.begin(), params.inDynamicAllocComponentNames.end());
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.debugStringLinesTemplateFilePath, allComponentNames, parsedDebugStringLines);
	didSucceed &= ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(params.debugDrawLinesTemplateFilePath, allComponentNames, parsedDebugDrawLines);

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
		else if (cppLineText.find("^^^^^") != std::string::npos)
		{
			for (std::string parsedLine : parsedDynamicAllocDefinitionLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("*****") != std::string::npos)
		{
			for (std::string parsedLine : parsedRemoveComponentBitset)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("~~~~~") != std::string::npos)
		{
			for (std::string parsedLine : parsedRemoveComponentValues)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("?????") != std::string::npos)
		{
			for (std::string parsedLine : parsedRemoveComponentDynamicAlloc)
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
		else if (cppLineText.find("&&&&&") != std::string::npos)
		{
			for (std::string parsedLine : parsedDynamicAllocResetLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("%%%%%") != std::string::npos)
		{
			for (std::string parsedLine : parsedDebugStringLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("!!!!!") != std::string::npos)
		{
			for (std::string parsedLine : parsedDebugDrawLines)
			{
				outFileContents.push_back(parsedLine);
			}
		}
		else if (cppLineText.find("#&#&#") != std::string::npos)
		{
			for (std::string parsedLine : parsedOwningEntityIdLines)
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

bool ArgusComponentRegistryCodeGenerator::ParseComponentSizeTestsTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents)
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