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

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementation code."), ARGUS_FUNCNAME)

	ArgusCodeGeneratorUtil::CombinedComponentDataOutput combinedComponentData;
	ArgusCodeGeneratorUtil::CombineStaticAndDynamicComponentData(parsedComponentData, combinedComponentData);

	// Parse Implementation file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedImplementationCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseComponentImplementationCppFileTemplateWithReplacements(combinedComponentData, outParsedImplementationCppFileContents);

	// Construct a directory path to registry location and to tests location. 
	FString componentImplementationDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentImplementationDirectory.Append(s_componentImplementationsDirectorySuffix);
	FPaths::MakeStandardFilename(componentImplementationDirectory);
	const char* cStrComponentImplementationDirectory = ARGUS_FSTRING_TO_CHAR(componentImplementationDirectory);

	// Write out header and cpp files.
	for (int i = 0; i < outParsedImplementationCppFileContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentImplementationDirectory).append(outParsedImplementationCppFileContents[i].m_filename), outParsedImplementationCppFileContents[i].m_lines);
	}

	if (didSucceed)
	{
		DeleteObsoleteFiles(combinedComponentData, cStrComponentImplementationDirectory);
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS component implementation code."), ARGUS_FUNCNAME)
	}
}

bool ComponentImplementationGenerator::ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentImplementationsTemplateDirectorySuffix));
	std::string componentImplementationCppTemplateFilename = std::string(cStrTemplateDirectory).append(s_componentImplementationCppTemplateFilename);
	std::string perObservableTemplateFilename = std::string(cStrTemplateDirectory).append(s_perObservableTemplateFilename);
	std::string sharedFunctionalityTemplateFilename = std::string(cStrTemplateDirectory).append(s_sharedFunctionalityTemplateFilename);

	std::ifstream inImplementationStream = std::ifstream(componentImplementationCppTemplateFilename);
	const FString ueImplementationFilePath = FString(componentImplementationCppTemplateFilename.c_str());
	if (!inImplementationStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueImplementationFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		writeData.m_filename = parsedComponentData.m_componentNames[i];
		writeData.m_filename.append(s_componentCppSuffix);
		outParsedFileContents.push_back(writeData);
	}

	std::string lineText;
	while (std::getline(inImplementationStream, lineText))
	{
		if (lineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_useSharedFunctions && !parsedComponentData.m_componentInfo[i].m_hasObservables)
				{
					continue;
				}

				std::string perComponentLineText = lineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentLineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
		}
		else if (lineText.find("$$$$$") != std::string::npos)
		{
			// Read from definitions template
			std::ifstream inStream = std::ifstream(sharedFunctionalityTemplateFilename);
			const FString ueFilePath = FString(sharedFunctionalityTemplateFilename.c_str());
			if (!inStream.is_open())
			{
				UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
				return false;
			}

			std::vector<std::string> rawLines = std::vector<std::string>();
			std::string templateLineText;
			while (std::getline(inStream, templateLineText))
			{
				rawLines.push_back(templateLineText);
			}
			inStream.close();

			// Parse per component template into one section
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_useSharedFunctions)
				{
					continue;
				}

				for (int j = 0; j < rawLines.size(); ++j)
				{
					outParsedFileContents[i].m_lines.push_back(std::regex_replace(rawLines[j], std::regex("#####"), parsedComponentData.m_componentNames[i]));
				}
			}
		}
		else if (lineText.find("%%%%%") != std::string::npos)
		{
			// Read from definitions template
			std::ifstream inStream = std::ifstream(perObservableTemplateFilename);
			const FString ueFilePath = FString(perObservableTemplateFilename.c_str());
			if (!inStream.is_open())
			{
				UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
				return false;
			}

			std::vector<std::string> rawLines = std::vector<std::string>();
			std::string templateLineText;
			while (std::getline(inStream, templateLineText))
			{
				rawLines.push_back(templateLineText);
			}
			inStream.close();

			// Parse per component template into one section
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_hasObservables)
				{
					continue;
				}

				for (int j = 0; j < parsedComponentData.m_componentVariableData[i].size(); ++j)
				{
					if (!parsedComponentData.m_componentVariableData[i][j].m_isObservable)
					{
						continue;
					}

					for (int k = 0; k < rawLines.size(); ++k)
					{
						std::string finalizedText = std::regex_replace(rawLines[k], std::regex("#####"), parsedComponentData.m_componentNames[i]);
						finalizedText = std::regex_replace(finalizedText, std::regex("#&#&#"), parsedComponentData.m_componentVariableData[i][j].m_varName);
						finalizedText = std::regex_replace(finalizedText, std::regex("&&&&&"), parsedComponentData.m_componentVariableData[i][j].m_typeName.substr(1));

						outParsedFileContents[i].m_lines.push_back(finalizedText);
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(lineText);
			}
		}
	}

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