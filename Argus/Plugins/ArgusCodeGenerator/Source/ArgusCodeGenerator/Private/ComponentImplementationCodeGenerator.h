// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ComponentImplementationGenerator
{
public:
	static void GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);

private:
	static const char* s_componentImplementationsDirectorySuffix;
	static const char* s_dynamicAllocComponentImplementationsDirectorySuffix;
	static const char* s_componentImplementationCppTemplateFilename;
	static const char* s_dynamicAllocComponentImplementationCppTemplateFilename;
	static const char* s_perObservableTemplateFilename;
	static const char* s_sharedFunctionalityTemplateFilename;
	static const char* s_componentCppSuffix;
	static const char* s_componentImplementationsTemplateDirectorySuffix;

	static bool ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseDynamicAllocComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static void GeneratePerVariableResetText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	static void GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiArrayField(const std::string& variableName, const std::string& extraData, bool isSmoothed, std::vector<std::string>& outParsedVariableContents, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> elementFormattingFunction);
	static void FormatImGuiQueueField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiDequeField(const std::string& variableName, const std::string& extraData, bool isSmoothed, std::vector<std::string>& outParsedVariableContents, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)> elementFormattingFunction);
	static void FormatImGuiFloatField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiIntField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiRecordField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiFVectorField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiFVector2DField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiEnumField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiTimerField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiResourceSetField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiOptionalField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiSpawnEntityInfoField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
};