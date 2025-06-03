// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <filesystem>
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
	static const char* s_propertyObservableDelimiter;
	static const char* s_propertyObservableDeclarationDelimiter;
	static const char* s_uePropertyDelimiter;
	static const char* s_sharedFunctionDeclarationDelimiter;

	static FString GetProjectDirectory();
	static FString GetTemplateDirectory(const char* subdirectorySuffix);
	static FString GetComponentDefinitionsDirectory();
	static FString GetDynamicAllocComponentDefinitionsDirectory();
	static FString GetStaticDataRecordDefinitionsDirectory();
	static FString GetSystemArgDefinitionsDirectory();

	struct ParsedVariableData
	{
		std::string m_typeName = "";
		std::string m_varName = "";
		std::string m_defaultValue = "";
		std::string m_propertyMacro = "";
		std::string m_staticDataTypeName = "";
		bool m_isObservable = false;
	};

	struct PerComponentData
	{
		bool m_hasObservables = false;
		bool m_useSharedFunctions = false;
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

		std::vector<PerComponentData> m_componentInfo;
		std::vector<PerComponentData> m_dynamicAllocComponentInfo;
	};
	struct CombinedComponentDataOutput
	{
		std::vector<std::string> m_componentNames;
		std::vector<std::string> m_componentRegistryIncludeStatements;
		std::vector<std::string> m_componentDataAssetIncludeStatements;
		std::vector< std::vector<ParsedVariableData> > m_componentVariableData;
		std::vector<PerComponentData> m_componentInfo;
	};
	struct ParseStaticDataRecordsOutput
	{
		std::vector<std::string> m_staticDataIncludeStatements;
		std::vector<std::string> m_staticDataRecordNames;
		std::vector< std::vector<ParsedVariableData> > m_staticDataRecordVariableData;
	};
	struct ParseSystemArgDefinitionsOutput
	{
		std::vector<std::string> m_systemArgsIncludeStatements;
		std::vector<std::string> m_systemArgsNames;
		std::vector< std::vector<ParsedVariableData> > m_systemArgsVariableData;
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
	static bool ParseSystemArgDefinitions(ParseSystemArgDefinitionsOutput& output);
	static bool ParseSystemArgDefinitionFromFile(const std::string& filePath, ParseSystemArgDefinitionsOutput& output);

	static bool GetRawLinesFromFile(const std::string& filePath, std::vector<std::string>& outFileContents);
	static bool WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents);

	static void CombineStaticAndDynamicComponentData(const ParseComponentDataOutput& input, CombinedComponentDataOutput& output);
	static void DoPerObservableReplacements(const ParseComponentDataOutput& input, const std::vector<std::string>& rawFileContents, std::vector<FileWriteData>& outParsedFileContents);
	
	template<typename TOutput>
	static void DeleteObsoleteComponentDependentFiles(const TOutput& parsedComponentData, const char* componentDependentFilesDirectory, const char* excludedFile = nullptr)
	{
		const std::string finalizedComponentDataDefinitionsDirectory = std::string(componentDependentFilesDirectory);
		std::vector<std::string> filesToDelete = std::vector<std::string>();

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(finalizedComponentDataDefinitionsDirectory))
		{
			const std::string filePath = entry.path().string();
			bool foundValidComponentName = false;
			for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
			{
				const size_t componentNameIndex = filePath.find(parsedComponentData.m_componentNames[i]);
				size_t componentDataIndex = std::string::npos;
				if (excludedFile)
				{
					componentDataIndex = filePath.find(excludedFile);
				}

				if (componentNameIndex != std::string::npos || componentDataIndex != std::string::npos)
				{
					foundValidComponentName = true;
					break;
				}
			}

			if (foundValidComponentName)
			{
				continue;
			}

			filesToDelete.push_back(filePath);
		}

		for (int i = 0; i < filesToDelete.size(); ++i)
		{
			std::remove(filesToDelete[i].c_str());
		}
	}


private:
	static const char* s_componentDefinitionDirectoryName;
	static const char* s_componentDefinitionDirectorySuffix;
	static const char* s_dynamicAllocComponentDefinitionDirectoryName;
	static const char* s_dynamicAllocComponentDefinitionDirectorySuffix;
	static const char* s_staticDataRecordDefinitionsDirectoryName;
	static const char* s_staticDataRecordDefinitionsDirectorySuffix;
	static const char* s_systemArgDefinitionsDirectoryName;
	static const char* s_systemArgDefinitionsDirectorySuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_structDelimiter;
	static const char* s_argusAPIDelimiter;
	static const char* s_varDelimiter;

	static bool ParseStructDeclarations(std::string& lineText);
	static bool ParseComponentStructDeclarations(std::string lineText, const std::string& componentDataAssetIncludeStatement, ParseComponentDataOutput& output, bool isDynamicallyAllocated);
	static bool ParseSystemArgStructDeclarations(std::string lineText, const std::string& systemArgIncludeStatement, ParseSystemArgDefinitionsOutput& output);
	static bool ParsePropertyMacro(std::string lineText, std::vector < std::vector<ParsedVariableData> >& parsedVariableData);
	static bool ParseVariableDeclarations(std::string lineText, bool withProperty, std::vector < std::vector<ParsedVariableData> >& parsedVariableData, bool& hasObservables);
	static bool ParseJointPropertyAndDeclarationMacro(std::string lineText, std::vector < std::vector<ParsedVariableData> >& parsedVariableData, bool& hasObservables);
	static bool ParseRecordClassDeclarations(std::string lineText, ParseStaticDataRecordsOutput& output);
};