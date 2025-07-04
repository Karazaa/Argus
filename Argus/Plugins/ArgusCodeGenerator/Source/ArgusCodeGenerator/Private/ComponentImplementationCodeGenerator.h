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
	static bool GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiArrayField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D);
	static void FormatImGuiQueueField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D);
	static void FormatImGuiDequeField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents, const bool isFloat, const bool isInt, const bool isFVector, const bool isFVector2D);
	static void FormatImGuiFloatField(const std::string& variableName, std::string& outFormattedString);
	static void FormatImGuiIntField(const std::string& variableName, std::string& outFormattedString);
	static void FormatImGuiFVectorField(const std::string& variableName, std::string& outFormattedString);
	static void FormatImGuiFVector2DField(const std::string& variableName, std::string& outFormattedString);
};