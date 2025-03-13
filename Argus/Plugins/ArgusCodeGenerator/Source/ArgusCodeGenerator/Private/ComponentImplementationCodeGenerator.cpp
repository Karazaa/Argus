// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentImplementationCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ComponentImplementationGenerator::s_componentImplementationsDirectorySuffix = "Source/Argus/ECS/ComponentImplementations/";
const char* ComponentImplementationGenerator::s_componentImplementationCppTemplateFilename = "ComponentImplementationCppTemplate.txt";
const char* ComponentImplementationGenerator::s_perObservableTemplateFilename = "PerObservableTemplate.txt";
const char* ComponentImplementationGenerator::s_sharedFunctionalityTemplateFilename = "SharedFunctionalityTemplate.txt";
const char* ComponentImplementationGenerator::s_componentHeaderSuffix = ".h";
const char* ComponentImplementationGenerator::s_componentCppSuffix = ".cpp";
const char* ComponentImplementationGenerator::s_componentImplementationsTemplateDirectorySuffix = "ComponentImplementations/";

void ComponentImplementationGenerator::GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS component implementation code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Construct a directory path to data asset component templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentImplementationsTemplateDirectorySuffix));

	std::string componentImplementationCppTemplateFilename = std::string(cStrTemplateDirectory).append(s_componentImplementationCppTemplateFilename);
	std::string perObservableTemplateFilename = std::string(cStrTemplateDirectory).append(s_perObservableTemplateFilename);
	std::string sharedFunctionalityTemplateFilename = std::string(cStrTemplateDirectory).append(s_sharedFunctionalityTemplateFilename);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementation code."), ARGUS_FUNCNAME)

	ArgusCodeGeneratorUtil::CombinedComponentDataOutput combinedComponentData;
	ArgusCodeGeneratorUtil::CombineStaticAndDynamicComponentData(parsedComponentData, combinedComponentData);

	// Parse SharedFunctionality file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedSharedFunctionalityFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseSharedFunctionalityFileTemplateWithReplacements(combinedComponentData, sharedFunctionalityTemplateFilename, outParsedSharedFunctionalityFileContents);

	// Parse PerObservable file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedPerObservableFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParsePerObservableFileTemplateWithReplacements(combinedComponentData, perObservableTemplateFilename, outParsedPerObservableFileContents);

	// Parse Implementation file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedImplementationCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseComponentImplementationCppFileTemplateWithReplacements(combinedComponentData, componentImplementationCppTemplateFilename, outParsedImplementationCppFileContents);

	// Construct a directory path to registry location and to tests location. 
	FString componentImplementationDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentImplementationDirectory.Append(s_componentImplementationsDirectorySuffix);
	FPaths::MakeStandardFilename(componentImplementationDirectory);
	const char* cStrComponentImplementationDirectory = ARGUS_FSTRING_TO_CHAR(componentImplementationDirectory);

	// Write out header and cpp files.
	for (int i = 0; i < outParsedImplementationCppFileContents.size(); ++i)
	{
		// didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentImplementationDirectory).append(outParsedImplementationCppFileContents[i].m_filename), outParsedImplementationCppFileContents[i].m_lines);
	}

	if (didSucceed)
	{
		DeleteObsoleteFiles(combinedComponentData, cStrComponentImplementationDirectory);
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS component implementation code."), ARGUS_FUNCNAME)
	}
}

bool ComponentImplementationGenerator::ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	return true;
}

bool ComponentImplementationGenerator::ParsePerObservableFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	return true;
}

bool ComponentImplementationGenerator::ParseSharedFunctionalityFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	return true;
}

void ComponentImplementationGenerator::DeleteObsoleteFiles(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, const char* componentDataDirectory)
{
	return;
}