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
	static const char* s_componentDefinitionDirectorySuffix;
	static const char* s_componentRegistryDirectorySuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_argusComponentRegistryHeaderTemplateFilename;
	static const char* s_argusComponentRegistryCppTemplateFilename;
	static const char* s_componentHeaderTemplateFilename;
	static const char* s_componentCppTemplateDefinitionsFilename;
	static const char* s_componentCppTemplateFlushFilename;
	static const char* s_componentCppTemplateResetFilename;
	static const char* s_argusComponentRegistryHeaderFilename;
	static const char* s_argusComponentRegistryCppFilename;
	static const char* s_classDelimiter;
	static const char s_inheritanceDelimiter;
	static const char s_openBracketDelimiter;

	static void ParseComponentNamesFromFile(const std::string& filePath, std::vector<std::string>& outComponentNames);
	static void ParseIncludeStatementsFromFile(const std::string& filePath, std::vector<std::string>& outIncludeStatements);

	struct ParseComponentRegistryTemplateParams
	{
		std::string argusComponentRegistryHeaderTemplateFilePath;
		std::string componentHeaderTemplateFilePath;
		std::string argusComponentRegistryCppTemplateFilePath;
		std::string componentCppTemplateDefinitionsFilePath;
		std::string componentCppTemplateFlushFilePath;
		std::string componentCppTemplateResetFilePath;
		std::vector<std::string> inComponentNames;
		std::vector<std::string> inIncludeStatements;
	};
	static void ParseComponentRegistryHeaderTemplate(const ParseComponentRegistryTemplateParams& params, std::vector<std::string>& outFileContents);
	static void ParseComponentRegistryCppTemplate(const ParseComponentRegistryTemplateParams& params, std::vector<std::string>& outFileContents);
	static void ParseComponentSpecificTemplate(const std::string& filePath, const std::vector<std::string>& componentNames, std::vector<std::string>& outFileContents);
	static void WriteOutComponentRegistryHeader(const std::string& filePath, const std::vector<std::string>& inFileContents);
	static void WriteOutComponentRegistryCpp(const std::string& filePath, const std::vector<std::string>& inFileContents);
};