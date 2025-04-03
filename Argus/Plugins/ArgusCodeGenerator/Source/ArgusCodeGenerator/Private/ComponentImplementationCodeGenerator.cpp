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
const char* ComponentImplementationGenerator::s_componentCppSuffix = ".cpp";
const char* ComponentImplementationGenerator::s_componentImplementationsTemplateDirectorySuffix = "ComponentImplementations/";

void ComponentImplementationGenerator::GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS component implementation code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementation code."), ARGUS_FUNCNAME)

	// Parse Implementation file
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedImplementationCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseComponentImplementationCppFileTemplateWithReplacements(parsedComponentData, outParsedImplementationCppFileContents);

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
		ArgusCodeGeneratorUtil::DeleteObsoleteComponentDependentFiles(parsedComponentData, cStrComponentImplementationDirectory);
		UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Successfully wrote out Argus ECS component implementation code."), ARGUS_FUNCNAME)
	}
}

bool ComponentImplementationGenerator::ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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
				std::string perComponentLineText = lineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentLineText, std::regex("#####"), parsedComponentData.m_componentNames[i]));
			}
		}
		else if (lineText.find("$$$$$") != std::string::npos)
		{
			std::vector<std::string> rawLines = std::vector<std::string>();
			ArgusCodeGeneratorUtil::GetRawLinesFromFile(sharedFunctionalityTemplateFilename, rawLines);

			// Parse per component template into one section
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				if (!parsedComponentData.m_componentInfo[i].m_useSharedFunctions)
				{
					continue;
				}

				for (int j = 0; j < rawLines.size(); ++j)
				{
					if (rawLines[j].find("$$$$$") != std::string::npos)
					{
						outParsedFileContents[i].m_lines.push_back("\tif (ImGui::BeginTable(\"ComponentValues\", 2, ImGuiTableFlags_NoSavedSettings))");
						outParsedFileContents[i].m_lines.push_back("\t{");
						GeneratePerVariableImGuiText(parsedComponentData.m_componentVariableData[i], outParsedFileContents[i].m_lines);
						outParsedFileContents[i].m_lines.push_back("\t\tImGui::EndTable();");
						outParsedFileContents[i].m_lines.push_back("\t}");
					}
					else
					{
						outParsedFileContents[i].m_lines.push_back(std::regex_replace(rawLines[j], std::regex("#####"), parsedComponentData.m_componentNames[i]));
					}
				}
			}
		}
		else if (lineText.find("%%%%%") != std::string::npos)
		{
			std::vector<std::string> rawLines = std::vector<std::string>();
			ArgusCodeGeneratorUtil::GetRawLinesFromFile(perObservableTemplateFilename, rawLines);
			ArgusCodeGeneratorUtil::DoPerObservableReplacements(parsedComponentData, rawLines, outParsedFileContents);
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

bool ComponentImplementationGenerator::GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents)
{
	for (int i = 0; i < parsedVariableData.size(); ++i)
	{
		const size_t unsignedInteger8Token = parsedVariableData[i].m_typeName.find("uint8");
		const size_t unsignedInteger16Token = parsedVariableData[i].m_typeName.find("uint16");
		const size_t unsignedInteger32Token = parsedVariableData[i].m_typeName.find("uint32");
		const size_t signedInteger8Token = parsedVariableData[i].m_typeName.find("int8");
		const size_t signedInteger16Token = parsedVariableData[i].m_typeName.find("int16");
		const size_t signedInteger32Token = parsedVariableData[i].m_typeName.find("int32");
		const bool isInteger =	unsignedInteger8Token != std::string::npos ||
								unsignedInteger16Token != std::string::npos ||
								unsignedInteger32Token != std::string::npos ||
								signedInteger8Token != std::string::npos ||
								signedInteger16Token != std::string::npos ||
								signedInteger32Token != std::string::npos;

		const bool isFloat = parsedVariableData[i].m_typeName.find("float") != std::string::npos;
		const bool isSmoother = parsedVariableData[i].m_typeName.find("ExponentialDecaySmoother") != std::string::npos;
		const bool isOptional = parsedVariableData[i].m_typeName.find("TOptional") != std::string::npos;
		const bool isArray = parsedVariableData[i].m_typeName.find("TArray") != std::string::npos;
		const bool isQueue = parsedVariableData[i].m_typeName.find("ArgusQueue") != std::string::npos;
		const bool isVector = parsedVariableData[i].m_typeName.find("FVector") != std::string::npos;

		outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
		std::string variable = "\t\tImGui::Text(\"";
		variable.append(parsedVariableData[i].m_varName);
		variable.append("\");");
		outParsedVariableContents.push_back(variable);
		outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");

		if (isInteger)
		{
			if (isArray)
			{

			}
			else
			{
				std::string value = "";
				FormatImGuiIntField(parsedVariableData[i].m_varName, value);
				outParsedVariableContents.push_back(value);
			}
		}
		else if (isFloat)
		{
			if (isArray)
			{

			}
			else
			{
				std::string variableName = parsedVariableData[i].m_varName;
				if (isSmoother)
				{
					variableName.append(".GetValue()");
				}
				std::string value = "";
				FormatImGuiFloatField(variableName, value);
				outParsedVariableContents.push_back(value);
			}
		}
		else if (isVector)
		{
			if (isOptional)
			{
				std::string condition = "\t\tif (";
				condition.append(parsedVariableData[i].m_varName);
				condition.append(".IsSet())");
				outParsedVariableContents.push_back(condition);
				outParsedVariableContents.push_back("\t\t{");
				std::string variableName = parsedVariableData[i].m_varName;
				variableName.append(".GetValue()");
				std::string value = "\t";
				FormatImGuiFVectorField(variableName, value);
				outParsedVariableContents.push_back(value);
				outParsedVariableContents.push_back("\t\t}");
			}
			else if (isArray)
			{

			}
			else if (isQueue)
			{

			}
			else
			{
				std::string variableName = parsedVariableData[i].m_varName;
				if (isSmoother)
				{
					variableName.append(".GetValue()");
				}
				std::string value = "";
				FormatImGuiFVectorField(variableName, value);
				outParsedVariableContents.push_back(value);
			}
		}
	}
	return true;
}

void ComponentImplementationGenerator::FormatImGuiFloatField(const std::string& variableName, std::string& outFormattedString)
{
	outFormattedString.append("\t\tImGui::Text(\"%f\", ");
	outFormattedString.append(variableName);
	outFormattedString.append(");");
}

void ComponentImplementationGenerator::FormatImGuiIntField(const std::string& variableName, std::string& outFormattedString)
{
	outFormattedString.append("\t\tImGui::Text(\"%d\", ");
	outFormattedString.append(variableName);
	outFormattedString.append(");");
}

void ComponentImplementationGenerator::FormatImGuiFVectorField(const std::string& variableName, std::string& outFormattedString)
{
	outFormattedString.append("\t\tImGui::Text(\"{%f, %f, %f}\", ");
	outFormattedString.append(variableName);
	outFormattedString.append(".X");
	outFormattedString.append(", ");
	outFormattedString.append(variableName);
	outFormattedString.append(".Y");
	outFormattedString.append(", ");
	outFormattedString.append(variableName);
	outFormattedString.append(".Z");
	outFormattedString.append(");");
}
