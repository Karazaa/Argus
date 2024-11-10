// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"
#include <string>
#include <vector>

class ArgusComponentRegistryCodeGenerator 
{
public:
	static void GenerateComponentRegistryCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);

private:
	// Constants for file parsing
	static const char* s_componentRegistryDirectorySuffix;
	static const char* s_ecsTestsDirectorySuffix;
	static const char* s_argusComponentRegistryHeaderTemplateFilename;
	static const char* s_argusComponentRegistryCppTemplateFilename;
	static const char* s_componentHeaderTemplateFilename;
	static const char* s_dynamicAllocComponentHeaderTemplateFilename;
	static const char* s_componentCppTemplateDefinitionsFilename;
	static const char* s_dynamicAllocComponentCppTemplateDefinitionsFilename;
	static const char* s_componentCppTemplateFlushFilename;
	static const char* s_componentCppTemplateResetFilename;
	static const char* s_dynamicAllocComponentCppTemplateResetFilename;
	static const char* s_debugStringLinesTemplateFilename;
	static const char* s_argusComponentSizeTestsTemplateFilename;
	static const char* s_perComponentSizeTestsTemplateFilename;
	static const char* s_argusComponentRegistryHeaderFilename;
	static const char* s_argusComponentRegistryCppFilename;
	static const char* s_argusComponentSizeTestsFilename;
	static const char* s_componentRegistryTemplateDirectorySuffix;

	struct ParseComponentTemplateParams
	{
		std::string argusComponentRegistryHeaderTemplateFilePath = "";
		std::string componentHeaderTemplateFilePath = "";
		std::string dynamicAllocComponentHeaderTemplateFilePath = "";
		std::string argusComponentRegistryCppTemplateFilePath = "";
		std::string componentCppTemplateDefinitionsFilePath = "";
		std::string dynamicAllocComponentCppTemplateDefinitionsFilePath = "";
		std::string componentCppTemplateFlushFilePath = "";
		std::string componentCppTemplateResetFilePath = "";
		std::string dynamicAllocComponentCppTemplateResetFilePath = "";
		std::string debugStringLinesTemplateFilePath = "";
		std::string argusComponentSizeTestsTemplateFilePath = "";
		std::string perComponentSizeTestsTemplateFilePath = "";
		std::vector<std::string> inComponentNames = std::vector<std::string>();
		std::vector<std::string> inDynamicAllocComponentNames = std::vector<std::string>();
		std::vector<std::string> inIncludeStatements = std::vector<std::string>();
		std::vector<std::string> inDynamicAllocIncludeStatements = std::vector<std::string>();
	};
	static bool ParseComponentRegistryHeaderTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
	static bool ParseComponentRegistryCppTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
	static bool ParseComponentSizeTestsTemplateWithReplacements(const ParseComponentTemplateParams& params, std::vector<std::string>& outFileContents);
};