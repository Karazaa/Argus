// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusStaticDataCodeGenerator
{
public:
	static void GenerateStaticDataCode(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords);

private:
	static const char* s_staticDataTemplateDirectorySuffix;
	static const char* s_staticDataDirectorySuffix;
	static const char* s_utilitiesDirectorySuffix;
	static const char* s_argusStaticDataTemplateFileName;
	static const char* s_argusStaticDataPerRecordTemplateFileName;
	static const char* s_argusStaticDataPerRecordEditorTemplateFileName;
	static const char* s_argusStaticDataFileName;
	static const char* s_argusStaticDatabaseHeaderTemplateFileName;
	static const char* s_argusStaticDatabaseHeaderPerRecordTemplateFileName;
	static const char* s_argusStaticDatabaseHeaderFileName;
	static const char* s_argusStaticDatabaseCppTemplateFileName;
	static const char* s_argusStaticDatabaseCppPerRecordTemplateFileName;
	static const char* s_argusStaticDatabaseCppFileName;
	static const char* s_recordDatabaseDirectorySuffix;
	static const char* s_recordDatabaseHeaderTemplateFileName;
	static const char* s_recordDatabaseCppTemplateFileName;
	static const char* s_recordDatabaseFileNameSuffix;
	static const char* s_softPtrLoadStoreHeaderTemplateFileName;
	static const char* s_softPtrLoadStorePerTypeHeaderTemplateFileName;
	static const char* s_softPtrLoadStoreHeaderFileName;
	static const char* s_softPtrLoadStoreCppTemplateFileName;
	static const char* s_softPtrLoadStorePerTypeCppTemplateFileName;
	static const char* s_softPtrLoadStoreCppFileName;

	static constexpr int k_numSoftPtrLoadStoreTypes = 3;
	static const char* s_softPtrLoadStoreTypeNames[k_numSoftPtrLoadStoreTypes];
	static const char* s_softPtrLoadStoreIncludes[k_numSoftPtrLoadStoreTypes];

	struct ParseTemplateParams
	{
		std::string templateFilePath = "";
		std::string perRecordTemplateFilePath = "";
		std::string perRecordEditorTemplateFilePath = "";
	};

	static bool ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseRecordDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents, std::vector<std::string>& outHeaderFilePaths);
	static bool ParseRecordDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParsePerRecordEditorTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParseSoftPtrLoadStoreTemplate(const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParseSoftPtrLoadStorePerTypeTemplate(const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
};