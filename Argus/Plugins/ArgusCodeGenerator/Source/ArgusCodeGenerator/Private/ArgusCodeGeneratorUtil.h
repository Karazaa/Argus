// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

DECLARE_LOG_CATEGORY_EXTERN(ArgusCodeGeneratorLog, Display, All);

class ArgusCodeGeneratorUtil
{
public:
	static FString GetProjectDirectory();
	static FString GetComponentDefinitionsDirectory();

	struct ParseComponentDataOutput
	{
		std::vector<std::string> m_componentNames;
		std::vector<std::string> m_componentRegistryIncludeStatements;
	};
	static bool ParseComponentData(ParseComponentDataOutput& output);
	static bool ParseComponentDataFromFile(const std::string& filePath, ParseComponentDataOutput& output);
	static bool ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents);
	static bool WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents);

private:
	static const char* s_componentDefinitionDirectoryName;
	static const char* s_componentDefinitionDirectorySuffix;
	static const char* s_structDelimiter;
};