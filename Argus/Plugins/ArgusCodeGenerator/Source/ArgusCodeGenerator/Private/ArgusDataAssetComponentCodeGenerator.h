// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusDataAssetComponentCodeGenerator
{
public:
	static void GenerateDataAssetComponentsCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);

private:
	static const char* s_componentDataDirectorySuffix;
	static const char* s_componentDataHeaderTemplateFilename;
	static const char* s_componentDataCppTemplateFilename;
	static const char* s_componentDataHeaderSuffix;
	static const char* s_componentDataCppSuffix;
	static const char* s_propertyMacro;
	static const char* s_upropertyPrefix;
	static const char* s_dataAssetComponentTemplateDirectorySuffix;

	static bool ParseDataAssetHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseDataAssetCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static void DeleteObsoleteFiles(const ArgusCodeGeneratorUtil::CombinedComponentDataOutput& parsedComponentData, const char* componentDataDirectory);
};