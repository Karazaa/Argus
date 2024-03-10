// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include <string>
#include <vector>

class ArgusComponentRegistryCodeGenerator 
{
public:
	static void GenerateComponentRegistry();
private:
	// Constants for file parsing
	static const char* s_componentDefinitionDirectoryName;
	static const char* s_componentDefinitionSuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_argusComponentRegistryHeaderTemplateFilename;
	static const char* s_argusComponentRegistryCppTemplateFilename;
	static const char* s_componentHeaderTemplateFilename;
	static const char* s_componentCppTemplateFilename;
	static const char* s_classDelimiter;
	static const char s_inheritanceDelimiter;
	static const char s_openBracketDelimiter;

	static void ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames);
	static void ParseIncludeStatementsFromFile(const std::string& filePath, std::vector<std::string>& outIncludeStatements);

	struct ParseComponentRegistryHeaderTemplateParams
	{
		std::string argusComponentRegistryHeaderTemplateFilePath;
		std::string componentHeaderTemplateFilePath;
		std::vector<std::string> inComponentNames;
		std::vector<std::string> inIncludeStatements;
	};
	static void ParseComponentRegistryHeaderTemplate(const ParseComponentRegistryHeaderTemplateParams& params, std::vector<std::string>& outFileContents);
};