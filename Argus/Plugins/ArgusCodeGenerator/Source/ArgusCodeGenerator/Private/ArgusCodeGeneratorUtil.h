// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <vector>

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

DECLARE_LOG_CATEGORY_EXTERN(ArgusCodeGeneratorLog, Display, All);

class ArgusCodeGeneratorUtil
{
public:
	static FString GetProjectDirectory();
	static FString GetTemplateDirectory();
	static FString GetComponentDefinitionsDirectory();

	struct ComponentVariableData
	{
		std::string m_typeName = "";
		std::string m_varName = "";
		std::string m_defaultValue = "";
	};
	struct ParseComponentDataOutput
	{
		std::vector<std::string> m_componentNames;
		std::vector<std::string> m_componentRegistryIncludeStatements;
		std::vector<std::string> m_componentDataAssetIncludeStatements;
		std::vector< std::vector<ComponentVariableData> > m_componentVariableData;
	};
	static bool ParseComponentData(ParseComponentDataOutput& output);
	static bool ParseComponentDataFromFile(const std::string& filePath, ParseComponentDataOutput& output);
	static bool ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents);
	static bool WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents);

private:
	static const char* s_componentDefinitionDirectoryName;
	static const char* s_componentDefinitionDirectorySuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_structDelimiter;
	static const char* s_varDelimiter;

	static bool ParseStructDeclarations(std::string lineText, const std::string& componentDataAssetIncludeStatement, ParseComponentDataOutput& output);
	static bool ParseVariableDeclarations(std::string lineText, ParseComponentDataOutput& output);
};