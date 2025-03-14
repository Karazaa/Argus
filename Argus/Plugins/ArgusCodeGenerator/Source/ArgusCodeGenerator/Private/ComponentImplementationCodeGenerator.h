// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ComponentImplementationGenerator
{
public:
	static void GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);

private:
	static const char* s_componentImplementationsDirectorySuffix;
	static const char* s_componentImplementationCppTemplateFilename;
	static const char* s_perObservableTemplateFilename;
	static const char* s_sharedFunctionalityTemplateFilename;
	static const char* s_componentCppSuffix;
	static const char* s_componentImplementationsTemplateDirectorySuffix;

	static bool ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static void DeleteObsoleteFiles(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const char* componentDataDirectory);
};