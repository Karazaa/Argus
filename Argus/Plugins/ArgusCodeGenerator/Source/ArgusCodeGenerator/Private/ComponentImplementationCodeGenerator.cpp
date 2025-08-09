// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentImplementationCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <regex>

const char* ComponentImplementationGenerator::s_componentImplementationsDirectorySuffix = "Source/Argus/ECS/ComponentImplementations/";
const char* ComponentImplementationGenerator::s_dynamicAllocComponentImplementationsDirectorySuffix = "Source/Argus/ECS/DynamicAllocComponentImplementations/";
const char* ComponentImplementationGenerator::s_componentImplementationCppTemplateFilename = "ComponentImplementationCppTemplate.txt";
const char* ComponentImplementationGenerator::s_dynamicAllocComponentImplementationCppTemplateFilename = "DynamicAllocComponentImplementationCppTemplate.txt";
const char* ComponentImplementationGenerator::s_perObservableTemplateFilename = "PerObservableTemplate.txt";
const char* ComponentImplementationGenerator::s_sharedFunctionalityTemplateFilename = "SharedFunctionalityTemplate.txt";
const char* ComponentImplementationGenerator::s_componentCppSuffix = ".cpp";
const char* ComponentImplementationGenerator::s_componentImplementationsTemplateDirectorySuffix = "ComponentImplementations/";

void ComponentImplementationGenerator::GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus ECS component implementation code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate component implementation code."), ARGUS_FUNCNAME)

	// Parse Implementation template files
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedImplementationCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseComponentImplementationCppFileTemplateWithReplacements(parsedComponentData, outParsedImplementationCppFileContents);

	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedDynamicAllocImplementationCppFileContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseDynamicAllocComponentImplementationCppFileTemplateWithReplacements(parsedComponentData, outParsedDynamicAllocImplementationCppFileContents);

	// Construct a directory path to static and dynamic alloc component implementation directories. 
	FString componentImplementationDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	componentImplementationDirectory.Append(s_componentImplementationsDirectorySuffix);
	FPaths::MakeStandardFilename(componentImplementationDirectory);
	const char* cStrComponentImplementationDirectory = ARGUS_FSTRING_TO_CHAR(componentImplementationDirectory);

	FString dynamicAllocComponentImplementationDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	dynamicAllocComponentImplementationDirectory.Append(s_dynamicAllocComponentImplementationsDirectorySuffix);
	FPaths::MakeStandardFilename(dynamicAllocComponentImplementationDirectory);
	const char* cStrDynamicAllocComponentImplementationDirectory = ARGUS_FSTRING_TO_CHAR(dynamicAllocComponentImplementationDirectory);

	// Write out static and dynamic alloc component cpp files.
	for (int i = 0; i < outParsedImplementationCppFileContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile
		(
			std::string(cStrComponentImplementationDirectory).append(outParsedImplementationCppFileContents[i].m_filename), 
			outParsedImplementationCppFileContents[i].m_lines
		);
	}
	for (int i = 0; i < outParsedDynamicAllocImplementationCppFileContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile
		(
			std::string(cStrDynamicAllocComponentImplementationDirectory).append(outParsedDynamicAllocImplementationCppFileContents[i].m_filename), 
			outParsedDynamicAllocImplementationCppFileContents[i].m_lines
		);
	}

	if (didSucceed)
	{
		ArgusCodeGeneratorUtil::DeleteObsoleteComponentDependentFiles(parsedComponentData.m_componentNames, cStrComponentImplementationDirectory);
		ArgusCodeGeneratorUtil::DeleteObsoleteComponentDependentFiles(parsedComponentData.m_dynamicAllocComponentNames, cStrDynamicAllocComponentImplementationDirectory);
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
					else if (rawLines[j].find("%%%%%") != std::string::npos)
					{
						GeneratePerVariableResetText(parsedComponentData.m_componentVariableData[i], outParsedFileContents[i].m_lines);
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

bool ComponentImplementationGenerator::ParseDynamicAllocComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_componentImplementationsTemplateDirectorySuffix));
	std::string componentImplementationCppTemplateFilename = std::string(cStrTemplateDirectory).append(s_dynamicAllocComponentImplementationCppTemplateFilename);

	std::ifstream inImplementationStream = std::ifstream(componentImplementationCppTemplateFilename);
	const FString ueImplementationFilePath = FString(componentImplementationCppTemplateFilename.c_str());
	if (!inImplementationStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueImplementationFilePath);
		return false;
	}

	for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		writeData.m_filename = parsedComponentData.m_dynamicAllocComponentNames[i];
		writeData.m_filename.append(s_componentCppSuffix);
		outParsedFileContents.push_back(writeData);
	}

	std::string lineText;
	while (std::getline(inImplementationStream, lineText))
	{
		if (lineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				std::string perComponentLineText = lineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perComponentLineText, std::regex("#####"), parsedComponentData.m_dynamicAllocComponentNames[i]));
			}
		}
		else if (lineText.find("$$$$$") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back("\tif (ImGui::BeginTable(\"ComponentValues\", 2, ImGuiTableFlags_NoSavedSettings))");
				outParsedFileContents[i].m_lines.push_back("\t{");
				GeneratePerVariableImGuiText(parsedComponentData.m_dynamicAllocComponentVariableData[i], outParsedFileContents[i].m_lines);
				outParsedFileContents[i].m_lines.push_back("\t\tImGui::EndTable();");
				outParsedFileContents[i].m_lines.push_back("\t}");
			}
		}
		else
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(lineText);
			}
		}
	}

	return true;
}

void ComponentImplementationGenerator::GeneratePerVariableResetText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents)
{
	for (int i = 0; i < parsedVariableData.size(); ++i)
	{
		if (!parsedVariableData[i].m_defaultValue.empty())
		{
			outParsedVariableContents.push_back(std::vformat("\t{} = {};", std::make_format_args(parsedVariableData[i].m_varName, parsedVariableData[i].m_defaultValue)));
			continue;
		}

		const bool isArray = parsedVariableData[i].m_typeName.find("TArray") != std::string::npos;
		const bool isDeque = parsedVariableData[i].m_typeName.find("ArgusDeque") != std::string::npos;
		const bool isResourceSet = parsedVariableData[i].m_typeName.find("FResourceSet") != std::string::npos;
		const bool isTimer = parsedVariableData[i].m_typeName.find("TimerHandle") != std::string::npos;
		const bool areObservers = parsedVariableData[i].m_typeName.find("Observers") != std::string::npos;
		if (isArray || isDeque || isResourceSet || isTimer || areObservers)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.Reset();", std::make_format_args(parsedVariableData[i].m_varName)));
			continue;
		}

		const bool isKDTreeOutput = parsedVariableData[i].m_typeName.find("ArgusEntityKDTreeRangeOutput") != std::string::npos;
		if (isKDTreeOutput)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.ResetAll();", std::make_format_args(parsedVariableData[i].m_varName)));
			continue;
		}

		const bool isSmoothed = parsedVariableData[i].m_typeName.find("ExponentialDecaySmoother") != std::string::npos;
		if (isSmoothed)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.ResetZero();", std::make_format_args(parsedVariableData[i].m_varName)));
			continue;
		}
	}
}

void ComponentImplementationGenerator::GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents)
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
		const bool isSmoothed = parsedVariableData[i].m_typeName.find("ExponentialDecaySmoother") != std::string::npos;
		const bool isOptional = parsedVariableData[i].m_typeName.find("TOptional") != std::string::npos;
		const bool isArray = parsedVariableData[i].m_typeName.find("TArray") != std::string::npos;
		const bool isQueue = parsedVariableData[i].m_typeName.find("ArgusQueue") != std::string::npos;
		const bool isDeque = parsedVariableData[i].m_typeName.find("ArgusDeque") != std::string::npos;
		const bool isVector2 = parsedVariableData[i].m_typeName.find("FVector2D") != std::string::npos;
		const bool isVector = parsedVariableData[i].m_typeName.find("FVector") != std::string::npos;
		const bool isTimer = parsedVariableData[i].m_typeName.find("TimerHandle") != std::string::npos;
		const bool isResourceSet = parsedVariableData[i].m_typeName.find("FResourceSet") != std::string::npos;
		const bool isStaticData = parsedVariableData[i].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter) != std::string::npos;
		const bool isKDTreeOutput = parsedVariableData[i].m_typeName.find("ArgusEntityKDTreeRangeOutput") != std::string::npos;
		const bool isSpawnEntityInfo = parsedVariableData[i].m_typeName.find("SpawnEntityInfo") != std::string::npos; 
		const bool isBool = parsedVariableData[i].m_typeName.find("bool") != std::string::npos;

		std::string cleanTypeName = parsedVariableData[i].m_typeName.substr(1, parsedVariableData[i].m_typeName.length() - 1);

		// BRUH REALLY? Yes. Until I find a better way to differentiate enum types... here we are.
		const bool isEnum = cleanTypeName.at(0) == 'E';

		if (!isKDTreeOutput)
		{
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
		}
		else
		{
			std::string avoidanceRangeName = std::vformat("{}.GetEntityIdsInAvoidanceRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(avoidanceRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(avoidanceRangeName, "", false, outParsedVariableContents, FormatImGuiIntField);

			std::string flockingRangeName = std::vformat("{}.GetEntityIdsInFlockingRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(flockingRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(flockingRangeName, "", false, outParsedVariableContents, FormatImGuiIntField);

			std::string sightRangeName = std::vformat("{}.GetEntityIdsInSightRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(sightRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(sightRangeName, "", false, outParsedVariableContents, FormatImGuiIntField);

			std::string rangedRangeName = std::vformat("{}.GetEntityIdsInRangedRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(rangedRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(rangedRangeName, "", false, outParsedVariableContents, FormatImGuiIntField);

			std::string meleeRangeName = std::vformat("{}.GetEntityIdsInMeleeRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(meleeRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(meleeRangeName, "", false, outParsedVariableContents, FormatImGuiIntField);

			return;
		}

		std::string extraData = "";
		TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> atomicFieldFormattingFunction;
		if (isOptional)
		{
			atomicFieldFormattingFunction = FormatImGuiOptionalField;
			if (isFloat)
			{
				extraData = "float";
			}
			else if (isVector)
			{
				extraData = "FVector";
			}
			else if (isVector2)
			{
				extraData = "FVector2D";
			}
		}
		else if (isInteger)
		{
			if (isStaticData)
			{
				extraData = parsedVariableData[i].m_propertyMacro;
				atomicFieldFormattingFunction = FormatImGuiRecordField;
			}
			else
			{
				atomicFieldFormattingFunction = FormatImGuiIntField;
			}
		}
		else if (isBool)
		{
			atomicFieldFormattingFunction = FormatImGuiBoolField;
		}
		else if (isFloat)
		{
			atomicFieldFormattingFunction = FormatImGuiFloatField;
		}
		else if (isVector2)
		{
			atomicFieldFormattingFunction = FormatImGuiFVector2DField;
		}
		else if (isVector)
		{
			atomicFieldFormattingFunction = FormatImGuiFVectorField;
		}
		else if (isEnum)
		{
			extraData = cleanTypeName;
			atomicFieldFormattingFunction = FormatImGuiEnumField;
		}
		else if (isTimer)
		{
			atomicFieldFormattingFunction = FormatImGuiTimerField;
		}
		else if (isResourceSet)
		{
			atomicFieldFormattingFunction = FormatImGuiResourceSetField;
		}
		else if (isSpawnEntityInfo)
		{
			atomicFieldFormattingFunction = FormatImGuiSpawnEntityInfoField;
		}

		if (isQueue)
		{
			FormatImGuiQueueField(parsedVariableData[i].m_varName, outParsedVariableContents);
		}
		else if (isArray)
		{
			FormatImGuiArrayField(parsedVariableData[i].m_varName, extraData, isSmoothed, outParsedVariableContents, atomicFieldFormattingFunction);
		}
		else if (isDeque)
		{
			FormatImGuiDequeField(parsedVariableData[i].m_varName, extraData, isSmoothed, outParsedVariableContents, atomicFieldFormattingFunction);
		}
		else if (atomicFieldFormattingFunction)
		{
			std::string variableName = parsedVariableData[i].m_varName;
			if (isSmoothed)
			{
				variableName.append(".GetValue()");
			}
			atomicFieldFormattingFunction(variableName, extraData, "", outParsedVariableContents);
		}
	}
	return;
}

void ComponentImplementationGenerator::FormatImGuiArrayField(const std::string& variableName, const std::string& extraData, bool isSmoothed, std::vector<std::string>& outParsedVariableContents, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> elementFormattingFunction)
{
	outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"Array max is currently = %d\", {}.Max());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.Num() == 0)", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Array is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"Size of array = %d\", {}.Num());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (int32 i = 0; i < {}.Num(); ++i)", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	std::string subVariableName = std::vformat("{}[i]", std::make_format_args(variableName));
	std::string prefix = "\t\t";
	if (elementFormattingFunction)
	{
		elementFormattingFunction(subVariableName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiQueueField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsEmpty())", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Queue is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Cannot traverse queue yet :(\");");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiDequeField(const std::string& variableName, const std::string& extraData, bool isSmoothed, std::vector<std::string>& outParsedVariableContents, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> elementFormattingFunction)
{
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsEmpty())", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Deque is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (auto element : {})", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	std::string subVariableName = "element";
	std::string prefix = "\t\t";
	if (elementFormattingFunction)
	{
		elementFormattingFunction(subVariableName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiFloatField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text(\"%.2f\", {});", std::make_format_args(prefix, variableName)));
}

void ComponentImplementationGenerator::FormatImGuiIntField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text(\"%d\", {});", std::make_format_args(prefix, variableName)));
}

void ComponentImplementationGenerator::FormatImGuiBoolField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text({} ? \"true\" : \"false\");", std::make_format_args(prefix, variableName)));
}

void ComponentImplementationGenerator::FormatImGuiRecordField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	const size_t lineSize = extraData.length();
	const size_t startIndex = extraData.find('(') + 1;
	std::string recordType = extraData.substr(startIndex, (lineSize - 1) - startIndex);

	std::string sanitizedVariableName = variableName;
	size_t delimiter = sanitizedVariableName.find('.');
	if (delimiter != std::string::npos)
	{
		sanitizedVariableName = sanitizedVariableName.substr((delimiter + 1), sanitizedVariableName.length() - (delimiter + 1));
	}

	outParsedVariableContents.push_back(std::vformat("{}\t\tif ({} != 0u)", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tif (const {}* record_{} = ArgusStaticData::GetRecord<{}>({}))",
		std::make_format_args(prefix, recordType, sanitizedVariableName, recordType, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tconst char* name_{} = ARGUS_FSTRING_TO_CHAR(record_{}->GetName());", std::make_format_args(prefix, sanitizedVariableName, sanitizedVariableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tImGui::Text(\"%s\", name_{});", std::make_format_args(prefix, sanitizedVariableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t}"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\telse"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tImGui::Text(\"None\");", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
}

void ComponentImplementationGenerator::FormatImGuiFVectorField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	std::string xValue = variableName;
	std::string yValue = variableName;
	std::string zValue = variableName;
	xValue.append(".X");
	yValue.append(".Y");
	zValue.append(".Z");

	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text(\"(%.2f, %.2f, %.2f)\", {}, {}, {});", std::make_format_args(prefix, xValue, yValue, zValue)));
}

void ComponentImplementationGenerator::FormatImGuiFVector2DField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	std::string xValue = variableName;
	std::string yValue = variableName;
	xValue.append(".X");
	yValue.append(".Y");

	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text(\"(%.2f, %.2f)\", {}, {});", std::make_format_args(prefix, xValue, yValue)));
}

void ComponentImplementationGenerator::FormatImGuiEnumField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	std::string newVariableName = "valueName_";
	newVariableName.append(variableName);
	outParsedVariableContents.push_back(std::vformat("{}\t\tconst char* {} = ARGUS_FSTRING_TO_CHAR(StaticEnum<{}>()->GetNameStringByValue(static_cast<uint8>({})))", std::make_format_args(prefix, newVariableName, extraData, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text({});", std::make_format_args(prefix, newVariableName)));
}

void ComponentImplementationGenerator::FormatImGuiTimerField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::string(prefix).append("\t\tconst ArgusEntity owningEntity = ArgusEntity::RetrieveEntity(GetOwningEntityId());"));
	outParsedVariableContents.push_back(std::vformat("{}\t\tif ({}.IsTimerTicking(owningEntity))", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tImGui::Text(\"%.2f\", {}.GetTimeRemaining(owningEntity));", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\tImGui::SameLine();"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tImGui::ProgressBar({}.GetTimeElapsedProportion(owningEntity));", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
	outParsedVariableContents.push_back(std::vformat("{}\t\telse if ({}.IsTimerComplete(owningEntity))", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\tImGui::Text(\"Timer complete\");"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\telse"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\tImGui::Text(\"Not set\");"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
}

void ComponentImplementationGenerator::FormatImGuiResourceSetField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tconst uint8 {}_numResources = static_cast<uint8>(EResourceType::Count);", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\tfor (int32 i = 0; i < {}_numResources; ++i)", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\tImGui::SameLine();"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tImGui::Text(\"%d \", {}.m_resourceQuantities[i]);", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
}

void ComponentImplementationGenerator::FormatImGuiOptionalField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tif ({}.IsSet())", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	std::string modifiedVariableName = variableName;
	modifiedVariableName.append(".GetValue()");
	std::string modifiedPrefix = prefix;
	modifiedPrefix.append("\t");

	if (extraData.find("float") != std::string::npos)
	{
		FormatImGuiFloatField(modifiedVariableName, "", modifiedPrefix, outParsedVariableContents);
	}
	else if (extraData.find("FVector") != std::string::npos)
	{
		FormatImGuiFVectorField(modifiedVariableName, "", modifiedPrefix, outParsedVariableContents);
	}
	else if (extraData.find("FVector2D") != std::string::npos)
	{
		FormatImGuiFVector2DField(modifiedVariableName, "", modifiedPrefix, outParsedVariableContents);
	}

	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\telse"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\tImGui::Text(\"Optional not set\");"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
}

void ComponentImplementationGenerator::FormatImGuiSpawnEntityInfoField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	std::string modifiedVariableName = variableName;
	modifiedVariableName.append(".m_argusActorRecordId");
	FormatImGuiRecordField(modifiedVariableName, "ARGUS_STATIC_DATA(UArgusActorRecord)", prefix, outParsedVariableContents);
}
