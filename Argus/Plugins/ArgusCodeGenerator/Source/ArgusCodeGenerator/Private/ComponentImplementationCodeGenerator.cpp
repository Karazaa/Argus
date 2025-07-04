// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ComponentImplementationCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <format>
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
		const bool isDeque = parsedVariableData[i].m_typeName.find("ArgusDeque") != std::string::npos;
		const bool isVector2 = parsedVariableData[i].m_typeName.find("FVector2D") != std::string::npos;
		const bool isVector = parsedVariableData[i].m_typeName.find("FVector") != std::string::npos;
		const bool isTimer = parsedVariableData[i].m_typeName.find("TimerHandle") != std::string::npos;
		const bool isResourceSet = parsedVariableData[i].m_typeName.find("FResourceSet") != std::string::npos;
		const bool isStaticData = parsedVariableData[i].m_propertyMacro.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter) != std::string::npos;
		const bool isKDTreeOutput = parsedVariableData[i].m_typeName.find("ArgusEntityKDTreeRangeOutput") != std::string::npos;
		const bool isSpawnEntityInfo = parsedVariableData[i].m_typeName.find("SpawnEntityInfo") != std::string::npos; 
		
		std::string cleanTypeName = parsedVariableData[i].m_typeName.substr(1, parsedVariableData[i].m_typeName.length() - 1);

		// BRUH REALLY? Yes. Until I find a better way to differentiate enum types... here we are.
		const bool isEnum = cleanTypeName.at(0) == 'E';

		if (!isKDTreeOutput)
		{
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
		}

		if (isInteger)
		{
			if (isStaticData)
			{
				const size_t lineSize = parsedVariableData[i].m_propertyMacro.length();
				const size_t startIndex = parsedVariableData[i].m_propertyMacro.find('(') + 1;
				std::string recordType = parsedVariableData[i].m_propertyMacro.substr(startIndex, (lineSize - 1) - startIndex);
				outParsedVariableContents.push_back(std::vformat("\t\tif ({} != 0u)", std::make_format_args(parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t{");
				outParsedVariableContents.push_back(std::vformat("\t\t\tif (const {}* record_{} = ArgusStaticData::GetRecord<{}>({}))", 
					std::make_format_args(recordType, parsedVariableData[i].m_varName, recordType, parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t\t{");
				outParsedVariableContents.push_back(std::vformat("\t\t\t\tconst char* name_{} = ARGUS_FSTRING_TO_CHAR(record_{}->GetName());", std::make_format_args(parsedVariableData[i].m_varName, parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back(std::vformat("\t\t\t\tImGui::Text(\"%s\", name_{});", std::make_format_args(parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t\t}");
				outParsedVariableContents.push_back("\t\t}");
				outParsedVariableContents.push_back("\t\telse");
				outParsedVariableContents.push_back("\t\t{");
				outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"None\", {});", std::make_format_args(parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t}");
			}
			else if (isArray)
			{
				FormatImGuiArrayField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
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
				FormatImGuiArrayField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
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
		else if (isEnum)
		{
			std::string newVariableName = "valueName_";
			newVariableName.append(parsedVariableData[i].m_varName);
			outParsedVariableContents.push_back(std::vformat("\t\tconst char* {} = ARGUS_FSTRING_TO_CHAR(StaticEnum<{}>()->GetNameStringByValue(static_cast<uint8>({})))", std::make_format_args(newVariableName, cleanTypeName, parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text({});", std::make_format_args(newVariableName)));
		}
		else if (isVector2)
		{
			if (isOptional)
			{
				outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsSet())", std::make_format_args(parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t{");
				std::string variableName = parsedVariableData[i].m_varName;
				variableName.append(".GetValue()");
				std::string value = "\t";
				FormatImGuiFVector2DField(variableName, value);
				outParsedVariableContents.push_back(value);
				outParsedVariableContents.push_back("\t\t}");
				outParsedVariableContents.push_back("\t\telse");
				outParsedVariableContents.push_back("\t\t{");
				outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Optional not set\");");
				outParsedVariableContents.push_back("\t\t}");
			}
			else if (isArray)
			{
				FormatImGuiArrayField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
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
				FormatImGuiFVector2DField(variableName, value);
				outParsedVariableContents.push_back(value);
			}

		}
		else if (isVector)
		{
			if (isOptional)
			{
				outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsSet())", std::make_format_args(parsedVariableData[i].m_varName)));
				outParsedVariableContents.push_back("\t\t{");
				std::string variableName = parsedVariableData[i].m_varName;
				variableName.append(".GetValue()");
				std::string value = "\t";
				FormatImGuiFVectorField(variableName, value);
				outParsedVariableContents.push_back(value);
				outParsedVariableContents.push_back("\t\t}");
				outParsedVariableContents.push_back("\t\telse");
				outParsedVariableContents.push_back("\t\t{");
				outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Optional not set\");");
				outParsedVariableContents.push_back("\t\t}");
			}
			else if (isArray)
			{
				FormatImGuiArrayField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
			}
			else if (isDeque)
			{
				FormatImGuiDequeField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
			}
			else if (isQueue)
			{
				FormatImGuiQueueField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
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
		else if (isTimer)
		{
			outParsedVariableContents.push_back("\t\tconst ArgusEntity owningEntity = ArgusEntity::RetrieveEntity(GetOwningEntityId());");
			outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsTimerTicking(owningEntity))", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t{");
			outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"%.2f\", {}.GetTimeRemaining(owningEntity));", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t\tImGui::SameLine();");
			outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::ProgressBar({}.GetTimeElapsedProportion(owningEntity));", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t}");
			outParsedVariableContents.push_back(std::vformat("\t\telse if ({}.IsTimerComplete(owningEntity))", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t{");
			outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Timer complete\");");
			outParsedVariableContents.push_back("\t\t}");
			outParsedVariableContents.push_back("\t\telse");
			outParsedVariableContents.push_back("\t\t{");
			outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Not set\");");
			outParsedVariableContents.push_back("\t\t}");
		}
		else if (isResourceSet)
		{
			outParsedVariableContents.push_back(std::vformat("\t\tconst uint8 {}_numResources = static_cast<uint8>(EResourceType::Count);", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back(std::vformat("\t\tfor (int32 i = 0; i < {}_numResources; ++i)", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t{");
			outParsedVariableContents.push_back("\t\t\tImGui::SameLine();");
			outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"%d \", {}.m_resourceQuantities[i]);", std::make_format_args(parsedVariableData[i].m_varName)));
			outParsedVariableContents.push_back("\t\t}");
		}
		else if (isKDTreeOutput)
		{
			std::string avoidanceRangeName = std::vformat("{}.GetEntitiesInAvoidanceRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(avoidanceRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(avoidanceRangeName, outParsedVariableContents, false, true, false, false);

			std::string sightRangeName = std::vformat("{}.GetEntitiesInSightRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(sightRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(sightRangeName, outParsedVariableContents, false, true, false, false);

			std::string rangedRangeName = std::vformat("{}.GetEntitiesInRangedRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(rangedRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(rangedRangeName, outParsedVariableContents, false, true, false, false);

			std::string meleeRangeName = std::vformat("{}.GetEntitiesInMeleeRange()", std::make_format_args(parsedVariableData[i].m_varName));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			outParsedVariableContents.push_back(std::vformat("\t\tImGui::Text(\"{}\");", std::make_format_args(meleeRangeName)));
			outParsedVariableContents.push_back("\t\tImGui::TableNextColumn();");
			FormatImGuiArrayField(meleeRangeName, outParsedVariableContents, false, true, false, false);
		}
		else if (isSpawnEntityInfo)
		{
			if (isDeque)
			{
				FormatImGuiDequeField(parsedVariableData[i].m_varName, outParsedVariableContents, isFloat, isInteger, isVector, isVector2);
			}
		}
	}
	return true;
}

void ComponentImplementationGenerator::FormatImGuiArrayField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D)
{
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.Num() == 0)", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Array is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tImGui::Text(\"Size of array = %d\", {}.Num());", std::make_format_args(variableName)));
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (int32 i = 0; i < {}.Num(); ++i)", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	std::string value = "\t\t";
	std::string subVariableName = std::vformat("{}[i]", std::make_format_args(variableName));
	if (isFloat)
	{
		FormatImGuiFloatField(subVariableName, value);
	}
	else if (isInt)
	{
		FormatImGuiIntField(subVariableName, value);
	}
	else if (isFVector)
	{
		FormatImGuiFVectorField(subVariableName, value);
	}
	else if (isFVector2D)
	{
		FormatImGuiFVector2DField(subVariableName, value);
	}
	outParsedVariableContents.push_back(value);
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiQueueField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D)
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

void ComponentImplementationGenerator::FormatImGuiDequeField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D)
{
	outParsedVariableContents.push_back(std::vformat("\t\tif ({}.IsEmpty())", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back("\t\t\tImGui::Text(\"Deque is empty\");");
	outParsedVariableContents.push_back("\t\t}");
	outParsedVariableContents.push_back("\t\telse");
	outParsedVariableContents.push_back("\t\t{");
	outParsedVariableContents.push_back(std::vformat("\t\t\tfor (auto element : {})", std::make_format_args(variableName)));
	outParsedVariableContents.push_back("\t\t\t{");
	std::string value = "\t\t";
	std::string subVariableName = "element";
	if (isFloat)
	{
		FormatImGuiFloatField(subVariableName, value);
	}
	else if (isInt)
	{
		FormatImGuiIntField(subVariableName, value);
	}
	else if (isFVector)
	{
		FormatImGuiFVectorField(subVariableName, value);
	}
	else if (isFVector2D)
	{
		FormatImGuiFVector2DField(subVariableName, value);
	}
	outParsedVariableContents.push_back(value);
	outParsedVariableContents.push_back("\t\t\t}");
	outParsedVariableContents.push_back("\t\t}");
}

void ComponentImplementationGenerator::FormatImGuiFloatField(const std::string& variableName, std::string& outFormattedString)
{
	outFormattedString.append(std::vformat("\t\tImGui::Text(\"%.2f\", {});", std::make_format_args(variableName)));
}

void ComponentImplementationGenerator::FormatImGuiIntField(const std::string& variableName, std::string& outFormattedString)
{
	outFormattedString.append(std::vformat("\t\tImGui::Text(\"%d\", {});", std::make_format_args(variableName)));
}

void ComponentImplementationGenerator::FormatImGuiFVectorField(const std::string& variableName, std::string& outFormattedString)
{
	std::string xValue = variableName;
	std::string yValue = variableName;
	std::string zValue = variableName;
	xValue.append(".X");
	yValue.append(".Y");
	zValue.append(".Z");

	outFormattedString.append(std::vformat("\t\tImGui::Text(\"(%.2f, %.2f, %.2f)\", {}, {}, {});", std::make_format_args(xValue, yValue, zValue)));
}

void ComponentImplementationGenerator::FormatImGuiFVector2DField(const std::string& variableName, std::string& outFormattedString)
{
	std::string xValue = variableName;
	std::string yValue = variableName;
	xValue.append(".X");
	yValue.append(".Y");

	outFormattedString.append(std::vformat("\t\tImGui::Text(\"(%.2f, %.2f)\", {}, {});", std::make_format_args(xValue, yValue)));
}
