// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusSystemArgsImplementationCodeGenerator
{
public:
	static void GenerateSystemArgsImplementation(const ArgusCodeGeneratorUtil::ParseSystemArgDefinitionsOutput& parsedSystemArgs);

	static const char* s_systemArgTemplateDirectorySuffix;
	static const char* s_systemArgImplementationTemplateFileName;
	static const char* s_systemArgImplementationDirectorySuffix;

private:
	static bool ParseSystemArgumentImplementationTemplate(const ArgusCodeGeneratorUtil::ParseSystemArgDefinitionsOutput& parsedSystemArgs, const std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static void TrimTypeName(std::string& typeName);
};