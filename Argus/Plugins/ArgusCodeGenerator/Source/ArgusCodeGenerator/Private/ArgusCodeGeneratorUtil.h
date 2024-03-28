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
	static bool ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents);
	static bool WriteOutFile(const std::string& filePath, const std::vector<std::string>& inFileContents);

private:
	static const char* s_componentDefinitionDirectorySuffix;
};