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
						outParsedFileContents[i].m_lines.push_back("\tif (ImGui::BeginTable(\"ComponentValues\", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInner))");
						outParsedFileContents[i].m_lines.push_back("\t{");
						GeneratePerVariableImGuiText(parsedComponentData.m_componentVariableData[i], outParsedFileContents[i].m_lines);
						outParsedFileContents[i].m_lines.push_back("\t\tImGui::EndTable();");
						outParsedFileContents[i].m_lines.push_back("\t}");

					}
					else if (rawLines[j].find("%%%%%") != std::string::npos)
					{
						GeneratePerVariableResetText(parsedComponentData.m_componentVariableData[i], outParsedFileContents[i].m_lines);
					}
					else if (rawLines[j].find("&&&&&") != std::string::npos)
					{
						GeneratePerVariableSerializeText(parsedComponentData.m_componentVariableData[i], outParsedFileContents[i].m_lines);
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
		else if (lineText.find("%%%%%") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				GeneratePerVariableResetText(parsedComponentData.m_dynamicAllocComponentVariableData[i], outParsedFileContents[i].m_lines);
			}
		}
		else if (lineText.find("$$$$$") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back("\tif (ImGui::BeginTable(\"ComponentValues\", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInner))");
				outParsedFileContents[i].m_lines.push_back("\t{");
				GeneratePerVariableImGuiText(parsedComponentData.m_dynamicAllocComponentVariableData[i], outParsedFileContents[i].m_lines);
				outParsedFileContents[i].m_lines.push_back("\t\tImGui::EndTable();");
				outParsedFileContents[i].m_lines.push_back("\t}");
			}
		}
		else if (lineText.find("&&&&&") != std::string::npos)
		{
			for (int i = 0; i < parsedComponentData.m_dynamicAllocComponentNames.size(); ++i)
			{
				GeneratePerVariableSerializeText(parsedComponentData.m_dynamicAllocComponentVariableData[i], outParsedFileContents[i].m_lines);
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
		if (parsedVariableData[i].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyNoResetDelimiter) != std::string::npos)
		{
			continue;
		}

		if (!parsedVariableData[i].m_defaultValue.empty())
		{
			outParsedVariableContents.push_back(std::vformat("\t{} = {};", std::make_format_args(parsedVariableData[i].m_varName, parsedVariableData[i].m_defaultValue)));
			continue;
		}

		const TypeInfo typeInfo = TypeInfo(parsedVariableData[i]);

		if (typeInfo.m_containerType == ContainerType::Array || typeInfo.m_containerType == ContainerType::BitArray || typeInfo.m_containerType == ContainerType::Deque || typeInfo.m_containerType == ContainerType::Set ||
			typeInfo.m_underlyingType == UnderlyingType::ResourceSet || typeInfo.m_underlyingType == UnderlyingType::TimerHandle || typeInfo.m_underlyingType == UnderlyingType::Observers)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.Reset();", std::make_format_args(typeInfo.m_cleanVariableName)));
			continue;
		}

		if (typeInfo.m_underlyingType == UnderlyingType::EntityKDTreeOutput || typeInfo.m_underlyingType == UnderlyingType::ObstacleKDTreeOutput)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.ResetAll();", std::make_format_args(typeInfo.m_cleanVariableName)));
			continue;
		}

		if (typeInfo.m_underlyingType == UnderlyingType::EntityKDTree || typeInfo.m_underlyingType == UnderlyingType::ObstacleKDTree)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.FlushAllNodes();", std::make_format_args(typeInfo.m_cleanVariableName)));
			continue;
		}

		if (typeInfo.m_containerType == ContainerType::ExponentialSmoother || typeInfo.m_containerType == SOSSmoother)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.ResetZero();", std::make_format_args(typeInfo.m_cleanVariableName)));
			continue;
		}

		if (typeInfo.m_containerType == ContainerType::Map)
		{
			outParsedVariableContents.push_back(std::vformat("\t{}.Empty();", std::make_format_args(typeInfo.m_cleanVariableName)));
			continue;
		}

		if (typeInfo.m_containerType == ContainerType::CArray)
		{
			outParsedVariableContents.push_back(std::vformat("\tfor (int32 i = 0; i < {}; ++i)", std::make_format_args(typeInfo.m_staticSize)));
			outParsedVariableContents.push_back("\t{");
			outParsedVariableContents.push_back(std::vformat("\t\t{}[i] = ArgusMath::GetZero<{}>();", std::make_format_args(typeInfo.m_cleanVariableName, typeInfo.m_cleanTypeName)));
			outParsedVariableContents.push_back("\t}");
			continue;
		}
	}
}

void ComponentImplementationGenerator::GeneratePerVariableSerializeText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents)
{
	for (int i = 0; i < parsedVariableData.size(); ++i)
	{
		if (parsedVariableData[i].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyTransientDelimiter) != std::string::npos)
		{
			continue;
		}

		const TypeInfo typeInfo = TypeInfo(parsedVariableData[i]);

		if (typeInfo.m_containerType == NoContainer)
		{
			switch (typeInfo.m_underlyingType)
			{
				case UnderlyingType::Bool:
				case UnderlyingType::Float:
				case UnderlyingType::Integer:
				case UnderlyingType::Vector:
				case UnderlyingType::Vector2:
				case UnderlyingType::Enum:
				case UnderlyingType::Bitmask:
				case UnderlyingType::StaticData:
					outParsedVariableContents.push_back(std::vformat("\tarchive << {};", std::make_format_args(typeInfo.m_cleanVariableName)));
					break;
				case UnderlyingType::ResourceSet:
				case UnderlyingType::TimerHandle:
				case UnderlyingType::NavAgentSelector:
					outParsedVariableContents.push_back(std::vformat("\t{}.Serialize(archive);", std::make_format_args(typeInfo.m_cleanVariableName)));
					break;
				default:
					break;
			}

			continue;
		}

		switch (typeInfo.m_containerType)
		{
			case ContainerType::ExponentialSmoother:
			case ContainerType::SOSSmoother:
			case ContainerType::BitArray:
			case ContainerType::Optional:
			case ContainerType::Deque:
				outParsedVariableContents.push_back(std::vformat("\t{}.Serialize(archive);", std::make_format_args(typeInfo.m_cleanVariableName)));
				break;
			case ContainerType::Array:
			case ContainerType::Map:
			case ContainerType::Set:
				outParsedVariableContents.push_back(std::vformat("\tarchive << {};", std::make_format_args(typeInfo.m_cleanVariableName)));
				break;
			case ContainerType::CArray:
				outParsedVariableContents.push_back(std::vformat("\tfor (int32 i = 0; i < {}; ++i)", std::make_format_args(typeInfo.m_staticSize)));
				outParsedVariableContents.push_back("\t{");
				outParsedVariableContents.push_back(std::vformat("\t\tarchive << {}[i];", std::make_format_args(typeInfo.m_cleanVariableName)));
				outParsedVariableContents.push_back("\t}");
				break;
			default:
				break;
		}
	}
}

void ComponentImplementationGenerator::GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents)
{
	for (int i = 0; i < parsedVariableData.size(); ++i)
	{
		const TypeInfo typeInfo = TypeInfo(parsedVariableData[i]);

		if (typeInfo.m_underlyingType == UnderlyingType::EntityKDTreeOutput)
		{
			std::string avoidanceRangeName = std::vformat("{}.GetEntityIdsInAvoidanceRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(avoidanceRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(avoidanceRangeName, "", outParsedVariableContents, FormatImGuiIntField);

			std::string groupExitRangeName = std::vformat("{}.GetEntityIdsInGroupExitRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(groupExitRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(groupExitRangeName, "", outParsedVariableContents, FormatImGuiIntField);

			std::string sightRangeName = std::vformat("{}.GetEntityIdsInSightRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(sightRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(sightRangeName, "", outParsedVariableContents, FormatImGuiIntField);

			continue;
		}

		outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
		outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(typeInfo.m_cleanVariableName)));
		outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");

		std::string extraData = "";
		TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> atomicFieldFormattingFunction = nullptr;
		std::string secondExtraData = "";
		TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> secondAtomicFieldFormattingFunction = nullptr;

		if (typeInfo.HasTemplateParameters() && !PopulateAtomicFormattingFunction(typeInfo.GetTemplateParameter(0), typeInfo.m_cleanTypeName, parsedVariableData[i].m_propertyMacro, atomicFieldFormattingFunction, extraData))
		{
			continue;
		}

		switch (typeInfo.m_containerType)
		{
			case ContainerType::Queue:
				FormatImGuiQueueField(typeInfo.m_cleanVariableName, outParsedVariableContents);
				break;
			case ContainerType::Array:
				FormatImGuiArrayField(typeInfo.m_cleanVariableName, extraData, outParsedVariableContents, atomicFieldFormattingFunction);
				break;
			case ContainerType::CArray:
				if (!PopulateAtomicFormattingFunction(typeInfo.m_underlyingType, typeInfo.m_cleanTypeName, parsedVariableData[i].m_propertyMacro, atomicFieldFormattingFunction, extraData))
				{
					continue;
				}
				FormatImGuiCArrayField(typeInfo.m_cleanVariableName, typeInfo.m_staticSize, extraData, outParsedVariableContents, atomicFieldFormattingFunction);
				break;
			case ContainerType::Deque:
				FormatImGuiDequeField(typeInfo.m_cleanVariableName, extraData, outParsedVariableContents, atomicFieldFormattingFunction);
				break;
			case ContainerType::Map:
				if (!PopulateAtomicFormattingFunction(typeInfo.GetTemplateParameter(1), typeInfo.m_cleanTypeName, parsedVariableData[i].m_propertyMacro, secondAtomicFieldFormattingFunction, secondExtraData))
				{
					continue;
				}
				FormatImGuiMapField(typeInfo.m_cleanVariableName, extraData, secondExtraData, outParsedVariableContents, atomicFieldFormattingFunction, secondAtomicFieldFormattingFunction);
				break;
			case ContainerType::Set:
				FormatImGuiSetField(typeInfo.m_cleanVariableName, extraData, outParsedVariableContents, atomicFieldFormattingFunction);
				break;
			case ContainerType::Optional:
				atomicFieldFormattingFunction = FormatImGuiOptionalField;
				switch (typeInfo.m_underlyingType)
				{
				case UnderlyingType::Float:
					extraData = "float";
					break;
				case UnderlyingType::Vector:
					extraData = "FVector";
					break;
				case UnderlyingType::Vector2:
					extraData = "FVector2D";
					break;
				default:
					break;
				}
				atomicFieldFormattingFunction(typeInfo.m_cleanVariableName, extraData, "", outParsedVariableContents);
				break;
			default:
				{
					std::string variableName = typeInfo.m_cleanVariableName;
					if (typeInfo.m_containerType == ContainerType::ExponentialSmoother || typeInfo.m_containerType == ContainerType::SOSSmoother)
					{
						variableName.append(".GetValue()");
					}
					if (!PopulateAtomicFormattingFunction(typeInfo.m_underlyingType, typeInfo.m_cleanTypeName, parsedVariableData[i].m_propertyMacro, atomicFieldFormattingFunction, extraData))
					{
						continue;
					}
					atomicFieldFormattingFunction(variableName, extraData, "", outParsedVariableContents);
				}
				break;
		}
	}
}

bool ComponentImplementationGenerator::PopulateAtomicFormattingFunction(UnderlyingType variableType, const std::string& cleanType, const std::string& macro, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& functionToPopulate, std::string& extraDataToPopulate)
{
	switch (variableType)
	{
		case UnderlyingType::Bitmask:
			extraDataToPopulate = cleanType;
			functionToPopulate = FormatImGuiBitmaskField;
			break;
		case UnderlyingType::StaticData:
			extraDataToPopulate = macro;
			functionToPopulate = FormatImGuiRecordField;
			break;
		case UnderlyingType::Integer:
			functionToPopulate = FormatImGuiIntField;
			break;
		case UnderlyingType::Bool:
			functionToPopulate = FormatImGuiBoolField;
			break;
		case UnderlyingType::Float:
			functionToPopulate = FormatImGuiFloatField;
			break;
		case UnderlyingType::Vector2:
			functionToPopulate = FormatImGuiFVector2DField;
			break;
		case UnderlyingType::Vector:
			functionToPopulate = FormatImGuiFVectorField;
			break;
		case UnderlyingType::Enum:
			extraDataToPopulate = cleanType;
			functionToPopulate = FormatImGuiEnumField;
			break;
		case UnderlyingType::TimerHandle:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::ResourceSet:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::SpawnEntityInfo:
			functionToPopulate = FormatImGuiSpawnEntityInfoField;
			break;
		case UnderlyingType::ControlGroup:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::TeamCommanderPriority:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::ResourceExtractionData:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::ConstructionData:
			functionToPopulate = FormatImGuiCustomTypeField;
			break;
		case UnderlyingType::NavAgentSelector:
			functionToPopulate = FormatImGuiNavAgentSelectorField;
			break;
		default:
			// TODO JAMES: Set some breakpoints here and capture any data types we don't currently have represented.
			return false;
	}
	return true;
}

void ComponentImplementationGenerator::FormatImGuiArrayField(const std::string& variableName, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction)
{
	outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"Array max is currently = %d\", {}.Max());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsEmpty())", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Array is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"Size of array = %d\", {}.Num());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\tImGui::Indent();");
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (int32 i = 0; i < {}.Num(); ++i)", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	outParsedVariableContents.push_back("\t\t\t\tif (i != 0) ImGui::Separator();");
	std::string subVariableName = std::vformat("{}[i]", std::make_format_args(variableName));
	std::string prefix = "\t\t";
	if (elementFormattingFunction)
	{
		elementFormattingFunction(subVariableName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t\tImGui::Unindent();");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiCArrayField(const std::string& variableName, const std::string& size, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction)
{
	outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"Size of array = %d\", {});", std::make_format_args(size)));
	outParsedVariableContents.push_back("\t\tImGui::Indent();");
	outParsedVariableContents.push_back(std::vformat("\t\tfor (int32 i = 0; i < {}; ++i)", std::make_format_args(size)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tif (i != 0) ImGui::Separator();");
	std::string subVariableName = std::vformat("{}[i]", std::make_format_args(variableName));
	std::string prefix = "\t";
	if (elementFormattingFunction)
	{
		elementFormattingFunction(subVariableName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\tImGui::Unindent();");
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

void ComponentImplementationGenerator::FormatImGuiDequeField(const std::string& variableName, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction)
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

void ComponentImplementationGenerator::FormatImGuiMapField(	const std::string& variableName, const std::string& extraData, const std::string& secondExtraData, std::vector<std::string>& outParsedVariableContents, 
															const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& element0FormattingFunction,
															const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& element1FormattingFunction)
{
	std::string prefix = "\t";
	std::string keyName = "mapPair.Key";
	std::string valueName = "mapPair.Value";
	outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"Size of map = %d\", {}.Num());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\tImGui::Indent();");
	outParsedVariableContents.push_back(std::vformat("\t\tImGui::BeginTable(\"{}\", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner);", std::make_format_args(variableName)));
	outParsedVariableContents.push_back(std::vformat("\t\tfor (const auto& mapPair : {})", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::TableNextColumn();");
	if (element0FormattingFunction)
	{
		element0FormattingFunction(keyName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t\tImGui::TableNextColumn();");
	if (element1FormattingFunction)
	{
		element1FormattingFunction(valueName, secondExtraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\tImGui::EndTable();");
	outParsedVariableContents.push_back("\t\tImGui::Unindent();");
}

void ComponentImplementationGenerator::FormatImGuiSetField(const std::string& variableName, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction)
{
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsEmpty())", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Set is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"Size of set = %d\", {}.Num());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\tImGui::Indent();");
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (const auto& element : {})", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	outParsedVariableContents.push_back("\t\t\t\tif (i != 0) ImGui::Separator();");
	std::string subVariableName = std::vformat("{}[i]", std::make_format_args(variableName));
	std::string prefix = "\t\t";
	if (elementFormattingFunction)
	{
		elementFormattingFunction(subVariableName, extraData, prefix, outParsedVariableContents);
	}
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t\tImGui::Unindent();");
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

void ComponentImplementationGenerator::FormatImGuiBitmaskField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	const size_t size = extraData.length();
	const size_t splitIndex = extraData.find("_") + 1;
	std::string enumName = extraData.substr(splitIndex, size - splitIndex);

	outParsedVariableContents.push_back(std::vformat("{}\t\tuint8 enumSize_{} = sizeof({}) * 8u;", std::make_format_args(prefix, variableName, enumName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\tbool triggered_{} = false;", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\tfor (uint8 i = 0u; i < enumSize_{}; ++i)", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tuint32 enumValue_{} = 1 << i;", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tif ({} & enumValue_{})", std::make_format_args(prefix, variableName, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tif (triggered_{})", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t\t{"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\t\tImGui::SameLine();", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t\t}"));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tconst char* valueName_{} = ARGUS_FSTRING_TO_CHAR(StaticEnum<{}>()->GetNameStringByValue(enumValue_{}));", 
		std::make_format_args(prefix, variableName, enumName, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tImGui::Text(\"%s, \", valueName_{});", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\ttriggered_{} = true;", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t\t}"));
	outParsedVariableContents.push_back(std::string(prefix).append("\t\t}"));
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
	const size_t bracket = variableName.find("[");
	if (bracket != std::string::npos)
	{
		newVariableName.append(variableName.substr(0, bracket));
		const size_t dot = variableName.find(".");
		if (dot != std::string::npos)
		{
			newVariableName.append("_");
			newVariableName.append(variableName.substr(dot + 1, variableName.length() - (dot + 1)));
			for (int i = 0; i < newVariableName.length(); ++i)
			{
				if (newVariableName[i] == '.')
				{
					newVariableName[i] = '_';
				}
			}
		}
	}
	else
	{
		newVariableName.append(variableName);
	}
	
	outParsedVariableContents.push_back(std::vformat("{}\t\tconst char* {} = ARGUS_FSTRING_TO_CHAR(StaticEnum<{}>()->GetNameStringByValue(static_cast<uint8>({})));", std::make_format_args(prefix, newVariableName, extraData, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\tImGui::Text({});", std::make_format_args(prefix, newVariableName)));
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
	FormatImGuiRecordField(modifiedVariableName, "ARGUS_COMP_STATIC_DATA(UArgusActorRecord)", prefix, outParsedVariableContents);
}

void ComponentImplementationGenerator::FormatImGuiCustomTypeField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\t{}.DrawImGuiDebug();", std::make_format_args(prefix, variableName)));
}

void ComponentImplementationGenerator::FormatImGuiNavAgentSelectorField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents)
{
	outParsedVariableContents.push_back(std::vformat("{}\t\tif (!{}.ContainsAnyAgent())", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t{{", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tImGui::Text(\"None\");", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t}}", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\telse", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t{{", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\tfor (int32 i = 0; i < 16; ++i)", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t{{", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\tif ({}.Contains(i))", std::make_format_args(prefix, variableName)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\t{{", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\t\tImGui::Text(\"Agent Index %d\", i);", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\t\tbreak;", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t\t}}", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t\t}}", std::make_format_args(prefix)));
	outParsedVariableContents.push_back(std::vformat("{}\t\t}}", std::make_format_args(prefix)));
}
