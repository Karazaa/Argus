// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <vector>

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)
#define ARGUS_FSTRING_TO_CHAR(fstring) (ANSICHAR*)StringCast<ANSICHAR, 256>(static_cast<const TCHAR*>(*fstring)).Get();

DECLARE_LOG_CATEGORY_EXTERN(ArgusCodeGeneratorLog, Display, All);

class ArgusCodeGeneratorUtil
{
public:
	static const char* s_propertyDelimiter;
	static const char* s_propertyIgnoreDelimiter;
	static const char* s_propertyStaticDataDelimiter;
	static const char* s_uePropertyDelimiter;

	static FString GetProjectDirectory();
	static FString GetTemplateDirectory(const char* subdirectorySuffix);
	static FString GetComponentDefinitionsDirectory();
	static FString GetDynamicAllocComponentDefinitionsDirectory();
	static FString GetStaticDataRecordDefinitionsDirectory();

	struct ParsedVariableData
	{
		std::string m_typeName = "";
		std::string m_varName = "";
		std::string m_defaultValue = "";
		std::string m_propertyMacro = "";
		std::string m_staticDataTypeName = "";
	};
	struct ParseComponentDataOutput
	{
		std::vector<std::string> m_componentNames;
		std::vector<std::string> m_componentRegistryIncludeStatements;
		std::vector<std::string> m_componentDataAssetIncludeStatements;
		std::vector< std::vector<ParsedVariableData> > m_componentVariableData;

		std::vector<std::string> m_dynamicAllocComponentNames;
		std::vector<std::string> m_dynamicAllocComponentRegistryIncludeStatements;
		std::vector<std::string> m_dynamicAllocComponentDataAssetIncludeStatements;
		std::vector< std::vector<ParsedVariableData> > m_dynamicAllocComponentVariableData;
	};
	struct ParseStaticDataRecordsOutput
	{
		std::vector<std::string> m_staticDataIncludeStatements;
		std::vector<std::string> m_staticDataRecordNames;
		std::vector< std::vector<ParsedVariableData> > m_staticDataRecordVariableData;
	};
	struct FileWriteData
	{
		std::string m_filename = "";
		std::vector<std::string> m_lines = std::vector<std::string>();
	};

	static bool ParseComponentData(ParseComponentDataOutput& output);
	static bool ParseComponentDataFromFile(const std::string& filePath, ParseComponentDataOutput& output, bool isDynamicallyAllocated);
	static bool ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents);
	static bool ParseStaticDataRecords(ParseStaticDataRecordsOutput& output);
	static bool ParseStaticDataDataRecordsFromFile(const std::string& filePath, ParseStaticDataRecordsOutput& output);


	static bool WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents);

private:
	static const char* s_componentDefinitionDirectoryName;
	static const char* s_componentDefinitionDirectorySuffix;
	static const char* s_dynamicAllocComponentDefinitionDirectoryName;
	static const char* s_dynamicAllocComponentDefinitionDirectorySuffix;
	static const char* s_staticDataRecordDefinitionsDirectoryName;
	static const char* s_staticDataRecordDefinitionsDirectorySuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_structDelimiter;
	static const char* s_argusAPIDelimiter;
	static const char* s_varDelimiter;

	static bool ParseStructDeclarations(std::string lineText, const std::string& componentDataAssetIncludeStatement, ParseComponentDataOutput& output, bool isDynamicallyAllocated);
	static bool ParsePropertyMacro(std::string lineText, std::vector < std::vector<ParsedVariableData> >& parsedVariableData);
	static bool ParseVariableDeclarations(std::string lineText, bool withProperty, std::vector < std::vector<ParsedVariableData> >& parsedVariableData);
	static bool ParseRecordClassDeclarations(std::string lineText, ParseStaticDataRecordsOutput& output);
};