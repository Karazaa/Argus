// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorUtil.h"
#include "Misc/Paths.h"
#include <fstream>
#include <regex>

DEFINE_LOG_CATEGORY(ArgusCodeGeneratorLog);

const char* ArgusCodeGeneratorUtil::s_propertyDelimiter = "ARGUS_PROPERTY";
const char* ArgusCodeGeneratorUtil::s_propertyIgnoreDelimiter = "ARGUS_IGNORE";
const char* ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter = "ARGUS_STATIC_DATA";
const char* ArgusCodeGeneratorUtil::s_propertyObservableDelimiter = "ARGUS_OBSERVABLE";
const char* ArgusCodeGeneratorUtil::s_propertyObservableDeclarationDelimiter = "ARGUS_OBSERVABLE_DECLARATION";
const char* ArgusCodeGeneratorUtil::s_uePropertyDelimiter = "UPROPERTY";
const char* ArgusCodeGeneratorUtil::s_sharedFunctionDeclarationDelimiter = "ARGUS_COMPONENT_SHARED";
const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectoryName = "ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_componentDefinitionDirectorySuffix = "Source/Argus/ECS/ComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_dynamicAllocComponentDefinitionDirectoryName = "DynamicAllocComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_dynamicAllocComponentDefinitionDirectorySuffix = "Source/Argus/ECS/DynamicAllocComponentDefinitions";
const char* ArgusCodeGeneratorUtil::s_staticDataRecordDefinitionsDirectoryName = "RecordDefinitions";
const char* ArgusCodeGeneratorUtil::s_staticDataRecordDefinitionsDirectorySuffix = "Source/Argus/StaticData/RecordDefinitions";
const char* ArgusCodeGeneratorUtil::s_templateDirectorySuffix = "Plugins/ArgusCodeGenerator/Source/ArgusCodeGenerator/Private/Templates/";
const char* ArgusCodeGeneratorUtil::s_structDelimiter = "struct ";
const char* ArgusCodeGeneratorUtil::s_argusAPIDelimiter = "ARGUS_API";
const char* ArgusCodeGeneratorUtil::s_varDelimiter = "m_";

FString ArgusCodeGeneratorUtil::GetProjectDirectory()
{
	FString projectDirectory = FPaths::GetProjectFilePath();
	FString cleanFilename = FPaths::GetCleanFilename(projectDirectory);
	projectDirectory.RemoveFromEnd(cleanFilename);

	return projectDirectory;
}

FString ArgusCodeGeneratorUtil::GetTemplateDirectory(const char* subdirectorySuffix)
{
	FString templateDirectory = *GetProjectDirectory();
	templateDirectory.Append(s_templateDirectorySuffix);
	templateDirectory.Append(subdirectorySuffix);
	FPaths::MakeStandardFilename(templateDirectory);

	return templateDirectory;
}

FString ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory()
{
	FString componentDefinitionsDirectory = *GetProjectDirectory();
	componentDefinitionsDirectory.Append(s_componentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(componentDefinitionsDirectory);

	return componentDefinitionsDirectory;
}

FString ArgusCodeGeneratorUtil::GetDynamicAllocComponentDefinitionsDirectory()
{
	FString dynamicAllocComponentDefinitionsDirectory = *GetProjectDirectory();
	dynamicAllocComponentDefinitionsDirectory.Append(s_dynamicAllocComponentDefinitionDirectorySuffix);
	FPaths::MakeStandardFilename(dynamicAllocComponentDefinitionsDirectory);

	return dynamicAllocComponentDefinitionsDirectory;
}

FString ArgusCodeGeneratorUtil::GetStaticDataRecordDefinitionsDirectory()
{
	FString recordDefinitionsDirectory = *GetProjectDirectory();
	recordDefinitionsDirectory.Append(s_staticDataRecordDefinitionsDirectorySuffix);
	FPaths::MakeStandardFilename(recordDefinitionsDirectory);

	return recordDefinitionsDirectory;
}

bool ArgusCodeGeneratorUtil::ParseComponentData(ParseComponentDataOutput& output)
{
	bool didSucceed = true;

	FString componentDefinitionsDirectory = ArgusCodeGeneratorUtil::GetComponentDefinitionsDirectory();
	const std::string finalizedComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*componentDefinitionsDirectory));

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		didSucceed &= ParseComponentDataFromFile(filePath, output, false);
	}

	FString dynamicAllocComponentDefinitionsDirectory = ArgusCodeGeneratorUtil::GetDynamicAllocComponentDefinitionsDirectory();
	const std::string finalizedDynamicAllocComponentDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*dynamicAllocComponentDefinitionsDirectory));

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedDynamicAllocComponentDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		didSucceed &= ParseComponentDataFromFile(filePath, output, true);
	}

	return didSucceed;
}

bool ArgusCodeGeneratorUtil::ParseComponentDataFromFile(const std::string& filePath, ParseComponentDataOutput& output, bool isDynamicallyAllocated)
{
	const size_t componentDefinitionIndex = filePath.find(isDynamicallyAllocated ? s_dynamicAllocComponentDefinitionDirectoryName : s_componentDefinitionDirectoryName);
	std::string includeStatement = "#include \"";
	std::string dataAssetIncludeStatement = "#include \"..\\";
	std::string componentPath = filePath.substr(componentDefinitionIndex);
	includeStatement.append(componentPath);
	includeStatement.append("\"");
	dataAssetIncludeStatement.append(componentPath);
	dataAssetIncludeStatement.append("\"");

	if (isDynamicallyAllocated)
	{
		output.m_dynamicAllocComponentRegistryIncludeStatements.push_back(includeStatement);
	}
	else
	{
		output.m_componentRegistryIncludeStatements.push_back(includeStatement);
	}

	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Reading from file: %s"), ARGUS_FUNCNAME, *ueFilePath);

	std::string lineText;
	bool didParsePropertyDeclaration = false;
	while (std::getline(inStream, lineText))
	{
		const size_t commentDelimeter = lineText.find("//");
		if (commentDelimeter != std::string::npos)
		{
			continue;
		}

		if (ParseStructDeclarations(lineText, dataAssetIncludeStatement, output, isDynamicallyAllocated))
		{
			continue;
		}

		std::vector < std::vector<ParsedVariableData> >& parsedVariableData = isDynamicallyAllocated ? output.m_dynamicAllocComponentVariableData : output.m_componentVariableData;
		std::vector<PerComponentData>& componentInfo = isDynamicallyAllocated ? output.m_dynamicAllocComponentInfo : output.m_componentInfo;

		const size_t sharedFunctionDeclarationDelimiter = lineText.find(s_sharedFunctionDeclarationDelimiter);
		if (sharedFunctionDeclarationDelimiter != std::string::npos)
		{
			componentInfo.back().m_useSharedFunctions = true;
			continue;
		}

		if (ParseJointPropertyAndDeclarationMacro(lineText, parsedVariableData, componentInfo.back().m_hasObservables))
		{
			didParsePropertyDeclaration = false;
			continue;
		}

		if (ParseVariableDeclarations(lineText, didParsePropertyDeclaration, parsedVariableData, componentInfo.back().m_hasObservables))
		{
			didParsePropertyDeclaration = false;
			continue;
		}

		if (ParsePropertyMacro(lineText, parsedVariableData))
		{
			didParsePropertyDeclaration = true;
			continue;
		}
	}
	inStream.close();
	return true;
}

bool ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents)
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

bool ArgusCodeGeneratorUtil::ParseStaticDataRecords(ParseStaticDataRecordsOutput& output)
{
	bool didSucceed = true;

	FString recordDefinitionsDirectory = ArgusCodeGeneratorUtil::GetStaticDataRecordDefinitionsDirectory();
	const std::string finalizedRecordDefinitionsDirectory = std::string(TCHAR_TO_UTF8(*recordDefinitionsDirectory));

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedRecordDefinitionsDirectory))
	{
		const std::string filePath = entry.path().string();
		didSucceed &= ParseStaticDataDataRecordsFromFile(filePath, output);
	}

	return didSucceed;
}

bool ArgusCodeGeneratorUtil::ParseStaticDataDataRecordsFromFile(const std::string& filePath, ParseStaticDataRecordsOutput& output)
{
	const size_t recordDefinitionIndex = filePath.find(s_staticDataRecordDefinitionsDirectoryName);
	std::string includeStatement = "#include \"";
	std::string recordPath = filePath.substr(recordDefinitionIndex);
	includeStatement.append(recordPath);
	includeStatement.append("\"");
	output.m_staticDataIncludeStatements.push_back(includeStatement);

	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Reading from file: %s"), ARGUS_FUNCNAME, *ueFilePath);

	std::string lineText;
	bool didParsePropertyDeclaration = false;
	bool hasObservables = false;
	while (std::getline(inStream, lineText))
	{
		if (ParseRecordClassDeclarations(lineText, output))
		{
			continue;
		}

		if (ParseVariableDeclarations(lineText, didParsePropertyDeclaration, output.m_staticDataRecordVariableData, hasObservables))
		{
			didParsePropertyDeclaration = false;
			continue;
		}

		if (ParsePropertyMacro(lineText, output.m_staticDataRecordVariableData))
		{
			didParsePropertyDeclaration = true;
			continue;
		}
	}
	inStream.close();
	return true;
}

bool ArgusCodeGeneratorUtil::GetRawLinesFromFile(const std::string& filePath, std::vector<std::string>& outFileContents)
{
	outFileContents.clear();

	// Read from definitions template
	std::ifstream inStream = std::ifstream(filePath);
	const FString ueFilePath = FString(filePath.c_str());
	if (!inStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueFilePath);
		return false;
	}

	std::string lineText;
	while (std::getline(inStream, lineText))
	{
		outFileContents.push_back(lineText);
	}
	inStream.close();
	return true;
}

bool ArgusCodeGeneratorUtil::WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents)
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

void ArgusCodeGeneratorUtil::CombineStaticAndDynamicComponentData(const ParseComponentDataOutput& input, CombinedComponentDataOutput& output)
{
	output.m_componentNames.clear();
	output.m_componentNames.reserve(input.m_componentNames.size() + input.m_dynamicAllocComponentNames.size());
	output.m_componentRegistryIncludeStatements.clear();
	output.m_componentRegistryIncludeStatements.reserve(input.m_componentRegistryIncludeStatements.size() + input.m_dynamicAllocComponentRegistryIncludeStatements.size());
	output.m_componentDataAssetIncludeStatements.clear();
	output.m_componentDataAssetIncludeStatements.reserve(input.m_componentDataAssetIncludeStatements.size() + input.m_dynamicAllocComponentDataAssetIncludeStatements.size());
	output.m_componentVariableData.clear();
	output.m_componentVariableData.reserve(input.m_componentVariableData.size() + input.m_dynamicAllocComponentVariableData.size());
	output.m_componentInfo.clear();
	output.m_componentInfo.reserve(input.m_componentInfo.size() + input.m_componentInfo.size());

	for (int i = 0; i < input.m_componentNames.size(); ++i)
	{
		output.m_componentNames.emplace_back(input.m_componentNames[i]);
	}
	for (int i = 0; i < input.m_dynamicAllocComponentNames.size(); ++i)
	{
		output.m_componentNames.emplace_back(input.m_dynamicAllocComponentNames[i]);
	}

	for (int i = 0; i < input.m_componentRegistryIncludeStatements.size(); ++i)
	{
		output.m_componentRegistryIncludeStatements.emplace_back(input.m_componentRegistryIncludeStatements[i]);
	}
	for (int i = 0; i < input.m_dynamicAllocComponentRegistryIncludeStatements.size(); ++i)
	{
		output.m_componentRegistryIncludeStatements.emplace_back(input.m_dynamicAllocComponentRegistryIncludeStatements[i]);
	}

	for (int i = 0; i < input.m_componentDataAssetIncludeStatements.size(); ++i)
	{
		output.m_componentDataAssetIncludeStatements.emplace_back(input.m_componentDataAssetIncludeStatements[i]);
	}
	for (int i = 0; i < input.m_dynamicAllocComponentDataAssetIncludeStatements.size(); ++i)
	{
		output.m_componentDataAssetIncludeStatements.emplace_back(input.m_dynamicAllocComponentDataAssetIncludeStatements[i]);
	}

	for (int i = 0; i < input.m_componentVariableData.size(); ++i)
	{
		output.m_componentVariableData.emplace_back(input.m_componentVariableData[i]);
	}
	for (int i = 0; i < input.m_dynamicAllocComponentVariableData.size(); ++i)
	{
		output.m_componentVariableData.emplace_back(input.m_dynamicAllocComponentVariableData[i]);
	}

	for (int i = 0; i < input.m_componentInfo.size(); ++i)
	{
		output.m_componentInfo.emplace_back(input.m_componentInfo[i]);
	}
	for (int i = 0; i < input.m_dynamicAllocComponentInfo.size(); ++i)
	{
		output.m_componentInfo.emplace_back(input.m_dynamicAllocComponentInfo[i]);
	}
}

void ArgusCodeGeneratorUtil::DoPerObservableReplacements(const ParseComponentDataOutput& input, const std::vector<std::string>& rawFileContents, std::vector<FileWriteData>& outParsedFileContents)
{
	for (int i = 0; i < input.m_componentNames.size(); ++i)
	{
		if (!input.m_componentInfo[i].m_hasObservables)
		{
			continue;
		}

		for (int j = 0; j < input.m_componentVariableData[i].size(); ++j)
		{
			if (!input.m_componentVariableData[i][j].m_isObservable)
			{
				continue;
			}

			for (int k = 0; k < rawFileContents.size(); ++k)
			{
				std::string finalizedText = std::regex_replace(rawFileContents[k], std::regex("#####"), input.m_componentNames[i]);
				finalizedText = std::regex_replace(finalizedText, std::regex("#&#&#"), input.m_componentVariableData[i][j].m_varName);
				finalizedText = std::regex_replace(finalizedText, std::regex("&&&&&"), input.m_componentVariableData[i][j].m_typeName.substr(1));

				outParsedFileContents[i].m_lines.push_back(finalizedText);
			}
		}
	}
}

bool ArgusCodeGeneratorUtil::ParseStructDeclarations(std::string lineText, const std::string& componentDataAssetIncludeStatement, ParseComponentDataOutput& output, bool isDynamicallyAllocated)
{
	const size_t structDelimiterLength = std::strlen(s_structDelimiter);
	const size_t structDelimiterIndex = lineText.find(s_structDelimiter);
	if (structDelimiterIndex == std::string::npos)
	{
		return false;
	}

	const size_t postClassStartIndex = structDelimiterIndex + structDelimiterLength;

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
	if (isDynamicallyAllocated)
	{
		output.m_dynamicAllocComponentNames.push_back(lineText);
		output.m_dynamicAllocComponentDataAssetIncludeStatements.push_back(componentDataAssetIncludeStatement);
		output.m_dynamicAllocComponentVariableData.push_back(std::vector<ParsedVariableData>());
		PerComponentData data;
		output.m_dynamicAllocComponentInfo.push_back(data);
	}
	else
	{
		output.m_componentNames.push_back(lineText);
		output.m_componentDataAssetIncludeStatements.push_back(componentDataAssetIncludeStatement);
		output.m_componentVariableData.push_back(std::vector<ParsedVariableData>());
		PerComponentData data;
		output.m_componentInfo.push_back(data);
	}
	return true;
}

bool ArgusCodeGeneratorUtil::ParsePropertyMacro(std::string lineText, std::vector < std::vector<ParsedVariableData> >& parsedVariableData)
{
	const size_t propertyDelimiterIndex = lineText.find(s_propertyDelimiter);
	const size_t propertyIgnoreDelimiterIndex = lineText.find(s_propertyIgnoreDelimiter);
	const size_t propertyStaticDataDelimiterIndex = lineText.find(s_propertyStaticDataDelimiter);
	const size_t propertyObservableDelimiter = lineText.find(s_propertyObservableDelimiter);
	const size_t uePropertyDelimiterIndex = lineText.find(s_uePropertyDelimiter);
	if (propertyDelimiterIndex == std::string::npos && 
		propertyIgnoreDelimiterIndex == std::string::npos &&
		propertyStaticDataDelimiterIndex == std::string::npos &&
		propertyObservableDelimiter == std::string::npos &&
		uePropertyDelimiterIndex == std::string::npos)
	{
		return false;
	}

	ParsedVariableData variableData;
	variableData.m_propertyMacro = lineText;
	variableData.m_isObservable = propertyObservableDelimiter != std::string::npos;
	parsedVariableData.back().push_back(variableData);

	return true;
}

bool ArgusCodeGeneratorUtil::ParseVariableDeclarations(std::string lineText, bool withProperty, std::vector < std::vector<ParsedVariableData> >& parsedVariableData, bool& hasObservables)
{
	const size_t variableDelimiterIndex = lineText.find(s_varDelimiter);
	if (variableDelimiterIndex == std::string::npos)
	{
		return false;
	}
	ParsedVariableData variableData;
	variableData.m_typeName = lineText.substr(0, variableDelimiterIndex);

	const size_t equalDelimiterIndex = lineText.find('=');
	const size_t endDelimiterIndex = lineText.find(';');
	if (equalDelimiterIndex != std::string::npos)
	{
		variableData.m_varName = lineText.substr(variableDelimiterIndex, equalDelimiterIndex - variableDelimiterIndex);
		if (endDelimiterIndex != std::string::npos)
		{
			variableData.m_defaultValue = lineText.substr(equalDelimiterIndex + 1, endDelimiterIndex - (equalDelimiterIndex + 1));
		}
		else
		{
			variableData.m_defaultValue = "";
		}
	}
	else if (endDelimiterIndex != std::string::npos)
	{
		variableData.m_varName = lineText.substr(variableDelimiterIndex, endDelimiterIndex - variableDelimiterIndex);
		variableData.m_defaultValue = "";
	}
	else
	{
		return false;
	}

	std::erase(variableData.m_typeName, ' ');
	std::erase(variableData.m_varName, ' ');
	std::erase(variableData.m_defaultValue, ' ');

	const int componentVariableCount = parsedVariableData.back().size();
	if (withProperty && componentVariableCount)
	{
		const int index = componentVariableCount - 1;
		parsedVariableData.back()[index].m_typeName = variableData.m_typeName;
		parsedVariableData.back()[index].m_varName = variableData.m_varName;
		parsedVariableData.back()[index].m_defaultValue = variableData.m_defaultValue;
		hasObservables |= parsedVariableData.back()[index].m_isObservable;
	}
	else
	{
		for (int i = 0; i < parsedVariableData.back().size(); ++i)
		{
			const size_t variableNameIndex = variableData.m_varName.find(parsedVariableData.back()[i].m_varName);
			if (variableNameIndex != std::string::npos)
			{
				return false;
			}
		}

		parsedVariableData.back().push_back(variableData);
	}
	return true;
}

bool ArgusCodeGeneratorUtil::ParseJointPropertyAndDeclarationMacro(std::string lineText, std::vector < std::vector<ParsedVariableData> >& parsedVariableData, bool& hasObservables)
{
	const size_t propertyObservableDeclarationDelimiter = lineText.find(s_propertyObservableDeclarationDelimiter);
	if (propertyObservableDeclarationDelimiter == std::string::npos)
	{
		return false;
	}

	const size_t observableDeclarationDelimiterLength = std::strlen(s_propertyObservableDeclarationDelimiter);
	const size_t startIndex = lineText.find('(') + 1;
	const size_t endIndex = lineText.find(')');
	lineText = lineText.substr(startIndex, endIndex - startIndex);
	
	const size_t firstCommaIndex = lineText.find_first_of(',');
	const size_t firstCommaIndexPlus2 = firstCommaIndex + 2;
	const size_t lastCommaIndex = lineText.find_last_of(',');
	const size_t lastCommaIndexPlus2 = lineText.find_last_of(',') + 2;

	ParsedVariableData variableData;
	variableData.m_typeName = "\t";
	variableData.m_typeName.append(lineText.substr(0, firstCommaIndex));
	variableData.m_varName = lineText.substr(firstCommaIndexPlus2, lastCommaIndex - firstCommaIndexPlus2);
	variableData.m_defaultValue = lineText.substr(lastCommaIndexPlus2, lineText.size() - lastCommaIndexPlus2);
	variableData.m_propertyMacro = s_propertyObservableDelimiter;
	variableData.m_isObservable = true;
	hasObservables = true;
	parsedVariableData.back().push_back(variableData);

	return true;
}

bool ArgusCodeGeneratorUtil::ParseRecordClassDeclarations(std::string lineText, ParseStaticDataRecordsOutput& output)
{
	const size_t argusAPIDelimiterLength = std::strlen(s_argusAPIDelimiter);
	const size_t argusAPIDelimiterIndex = lineText.find(s_argusAPIDelimiter);
	if (argusAPIDelimiterIndex == std::string::npos)
	{
		return false;
	}

	const size_t postClassStartIndex = argusAPIDelimiterIndex + argusAPIDelimiterLength;

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
	output.m_staticDataRecordNames.push_back(lineText);
	output.m_staticDataRecordVariableData.push_back(std::vector<ParsedVariableData>());
	return true;
}