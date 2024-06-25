// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once
#include "ArgusCodeGeneratorUtil.h"

class ArgusDataAssetComponentCodeGenerator
{
public:
	static void GenerateDataAssetComponents(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);
private:
	static const char* s_componentDataDirectorySuffix;
	static const char* s_componentDataHeaderTemplateFilename;
	static const char* s_componentDataCppTemplateFilename;
	static const char* s_componentDataHeaderSuffix;
	static const char* s_componentDataCppSuffix;
	static const char* s_propertyMacro;
	static const char* s_upropertyPrefix;
	static const char* s_dataAssetComponentTemplateDirectorySuffix;

	struct FileWriteData
	{
		std::string m_filename = "";
		std::vector<std::string> m_lines = std::vector<std::string>();
	};
	static bool ParseDataAssetHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<FileWriteData>& outParsedFileContents);
	static bool ParseDataAssetCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::string& templateFilePath, std::vector<FileWriteData>& outParsedFileContents);
	static void DeleteObsoleteFiles(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const char* componentDataDirectory);
};