// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"
#include <string>
#include <vector>

class ArgusComponentRegistryCodeGenerator 
{
public:
	static void GenerateComponentRegistry(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);
private:
	// Constants for file parsing
	static const char* s_componentRegistryDirectorySuffix;
	static const char* s_ecsTestsDirectorySuffix;
	static const char* s_templateDirectorySuffix;
	static const char* s_argusComponentRegistryHeaderTemplateFilename;
	static const char* s_argusComponentRegistryCppTemplateFilename;
	static const char* s_componentHeaderTemplateFilename;
	static const char* s_componentCppTemplateDefinitionsFilename;
	static const char* s_componentCppTemplateFlushFilename;
	static const char* s_componentCppTemplateResetFilename;
	static const char* s_argusComponentSizeTestsTemplateFilename;
	static const char* s_perComponentSizeTestsTemplateFilename;
	static const char* s_argusComponentRegistryHeaderFilename;
	static const char* s_argusComponentRegistryCppFilename;
	static const char* s_argusComponentSizeTestsFilename;

	struct ParseComponentTemplateParams
	{
		std::string argusComponentRegistryHeaderTemplateFilePath;
		std::string componentHeaderTemplateFilePath;
		std::string argusComponentRegistryCppTemplateFilePath;
		std::string componentCppTemplateDefinitionsFilePath;
		std::string componentCppTemplateFlushFilePath;
		std::string componentCppTemplateResetFilePath;
		std::string argusComponentSizeTestsTemplateFilePath;
		std::string perComponentSizeTestsTemplateFilePath;
		std::vector<std::string> inComponentNames;
		std::vector<std::string> inIncludeStatements;
	};
	static bool ParseComponentRegistryHeaderTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
	static bool ParseComponentRegistryCppTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
	static bool ParseComponentSizeTestsTemplate(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
};