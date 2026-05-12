// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class AssetFactoryGenerator
{
public:
	static void GenerateAssetFactories(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords);

private:

	static const char* s_componentAssetFactoriesDirectorySuffix;
	static const char* s_recordAssetFactoriesDirectorySuffix;
	static const char* s_argusEditorDirectorySuffix;
	static const char* s_componentAssetFactoriesHeaderTemplateFilename;
	static const char* s_recordAssetFactoriesHeaderTemplateFilename;
	static const char* s_componentAssetFactoriesCppTemplateFilename;
	static const char* s_recordAssetFactoriesCppTemplateFilename;
	static const char* s_argusEditorGeneratedHeaderTemplateFilename;
	static const char* s_argusEditorGeneratedCppTemplateFilename;
	static const char* s_assetActionsPerComponentTemplateFilename;
	static const char* s_assetActionsPerRecordTemplateFilename;
	static const char* s_assetFactoriesTemplateDirectorySuffix;

	static bool ParseComponentAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, const std::string& templateFilePath, bool isHeader, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseRecordAssetFactoryTemplateFile(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const std::string& templateFilePath, bool isHeader, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
};