// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentObserversCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ComponentObserversGenerator::s_componentObserversDirectorySuffix = "Source/Argus/ECS/ComponentObservers/";
const char* ComponentObserversGenerator::s_componentObserversHeaderTemplateFilename = "ComponentObserversHeaderTemplate.txt";
const char* ComponentObserversGenerator::s_perObservableAbstractFuncTemplateFilename = "PerObservableAbstractFunctionTemplate.txt";
const char* ComponentObserversGenerator::s_perObservableOnChangedTemplateFilename = "PerObservableOnChangedTemplate.txt";
const char* ComponentObserversGenerator::s_componentObserversHeaderSuffix = "Observers.h";
const char* ComponentObserversGenerator::s_componentObserversTemplateDirectorySuffix = "ComponentObservers/";
const char* ComponentObserversGenerator::s_observersComponentTemplateDirectorySuffix = "ObserversComponentTemplate.txt";
const char* ComponentObserversGenerator::s_perObservableTypeTemplateDirectorySuffix = "ObserversComponentPerTypeTemplate.txt";
const char* ComponentObserversGenerator::s_observersComponentFilename = "/ObserversComponent.h";

void ComponentObserversGenerator::GenerateObserversComponentCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of ObserversComponent.h."), ARGUS_FUNCNAME);
	bool didSucceed = true;

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate ObserversComponent.h."), ARGUS_FUNCNAME);

	std::vector<ArgusCodeGeneratorUtil::FileWriteData> observersComponentFileData;
	observersComponentFileData.resize(1);
	didSucceed &= ParseObserversComponentHeaderFileTemplateWithReplacements(parsedComponentData, observersComponentFileData);
	
	// Construct a directory path to componentdefinitions location. 
	const char* cStrComponentDefinitionsDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory());
	didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentDefinitionsDirectory).append(observersComponentFileData[0].m_filename), observersComponentFileData[0].m_lines);

	if (didSucceed)
	{
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out ObserversComponent.h."), ARGUS_FUNCNAME)
	}
}

void ComponentObserversGenerator::GenerateComponentObserversCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS component observers code."), ARGUS_FUNCNAME);
	bool didSucceed = true;

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component observers code."), ARGUS_FUNCNAME);

	// Parse header file file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedObserversHeaderFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseComponentObserversHeaderFileTemplateWithReplacements(parsedComponentData, outParsedObserversHeaderFileContents);

	// Construct a directory path to observables location.
	FString componentObserversDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentObserversDirectory.Append(s_componentObserversDirectorySuffix);
	FPaths::MakeStandardFilename(componentObserversDirectory);
	const char* cStrComponentObserversDirectory = ARGUS_FSTRING_TO_CHAR(componentObserversDirectory);

	// Write out header and cpp files.
	for (int i = 0; i < outParsedObserversHeaderFileContents.size(); ++i)
	{
		if (!outParsedObserversHeaderFileContents[i].IsEmpty())
		{
			didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrComponentObserversDirectory).append(outParsedObserversHeaderFileContents[i].m_filename), outParsedObserversHeaderFileContents[i].m_lines);
		}
	}

	if (didSucceed)
	{
		ArgusCodeGeneratorUtil::DeleteObsoleteComponentDependentFiles(parsedComponentData.m_componentNames, cStrComponentObserversDirectory);
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS component observers code."), ARGUS_FUNCNAME)
	}
}

bool ComponentObserversGenerator::ParseObserversComponentHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	if (outParsedFileContents.empty())
	{
		return false;
	}

	outParsedFileContents[0].m_filename = s_observersComponentFilename;

	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentObserversTemplateDirectorySuffix));
	std::string observersComponentHeaderTemplateFilename = std::string(cStrTemplateDirectory).append(s_observersComponentTemplateDirectorySuffix);
	std::string perObservableTypeTemplateFilename = std::string(cStrTemplateDirectory).append(s_perObservableTypeTemplateDirectorySuffix);

	std::ifstream inObserversStream = std::ifstream(observersComponentHeaderTemplateFilename);
	const FString ueObserversFilePath = FString(observersComponentHeaderTemplateFilename.c_str());
	if (!inObserversStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueObserversFilePath);
		return false;
	}

	std::vector<std::string> rawLines = std::vector<std::string>();
	ArgusCodeGeneratorUtil::GetRawLinesFromFile(perObservableTypeTemplateFilename, rawLines);

	std::string lineText;
	while (std::getline(inObserversStream, lineText))
	{
		if (lineText.find("$$$$$") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_hasObservables)
				{
					continue;
				}

				outParsedFileContents[0].m_lines.push_back(std::vformat("#include \"ComponentObservers/{}Observers.h\"", std::make_format_args(parsedComponentData.m_componentNames[i])));
			}
		}
		else if (lineText.find("%%%%%") != std::string::npos)
		{
			ArgusCodeGeneratorUtil::DoPerObservableReplacements(parsedComponentData, rawLines, outParsedFileContents);
		}
		else
		{
			outParsedFileContents[0].m_lines.push_back(lineText);
		}
	}

	return true;
}

bool ComponentObserversGenerator::ParseComponentObserversHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentObserversTemplateDirectorySuffix));
	std::string componentObserversHeaderTemplateFilename = std::string(cStrTemplateDirectory).append(s_componentObserversHeaderTemplateFilename);
	std::string perObservableAbstractFuncTemplateFilename = std::string(cStrTemplateDirectory).append(s_perObservableAbstractFuncTemplateFilename);
	std::string perObservableOnChangedTemplateFilename = std::string(cStrTemplateDirectory).append(s_perObservableOnChangedTemplateFilename);

	std::ifstream inObserversStream = std::ifstream(componentObserversHeaderTemplateFilename);
	const FString ueObserversFilePath = FString(componentObserversHeaderTemplateFilename.c_str());
	if (!inObserversStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueObserversFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		if (parsedComponentData.m_componentInfo[i].m_hasObservables)
		{
			writeData.m_filename = parsedComponentData.m_componentNames[i];
			writeData.m_filename.append(s_componentObserversHeaderSuffix);
		}
		outParsedFileContents.push_back(writeData);
	}

	std::vector<std::string> rawLinesAbstract = std::vector<std::string>();
	std::vector<std::string> rawLinesOnChanged = std::vector<std::string>();
	ArgusCodeGeneratorUtil::GetRawLinesFromFile(perObservableAbstractFuncTemplateFilename, rawLinesAbstract);
	ArgusCodeGeneratorUtil::GetRawLinesFromFile(perObservableOnChangedTemplateFilename, rawLinesOnChanged);

	std::string lineText;
	while (std::getline(inObserversStream, lineText))
	{
		if (lineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_hasObservables)
				{
					continue;
				}
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(lineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
		}
		else if (lineText.find("$$$$$") != std::string::npos)
		{
			ArgusCodeGeneratorUtil::DoPerObservableReplacements(parsedComponentData, rawLinesAbstract, outParsedFileContents);
		}
		else if (lineText.find("%%%%%") != std::string::npos)
		{
			ArgusCodeGeneratorUtil::DoPerObservableReplacements(parsedComponentData, rawLinesOnChanged, outParsedFileContents);
		}
		else
		{
			for (int i = 0; i < outParsedFileContents.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(lineText);
			}
		}
	}

	return true;
}