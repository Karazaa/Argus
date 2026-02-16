// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ArgusStaticDataCodeGenerator
{
public:
	static bool GenerateStaticDataCode(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords);

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
	static const char* s_recordReferenceDirectorySuffix;
	static const char* s_recordReferenceHeaderTemplateFileName;
	static const char* s_recordReferenceCppTemplateFileName;
	static const char* s_recordReferenceFileNameSuffix;
	static const char* s_softPtrLoadStoreHeaderTemplateFileName;
	static const char* s_softObjectLoadStorePerTypeHeaderTemplateFileName;
	static const char* s_softClassLoadStorePerTypeHeaderTemplateFileName;
	static const char* s_softPtrLoadStoreHeaderFileName;
	static const char* s_softPtrLoadStoreCppTemplateFileName;
	static const char* s_softObjectLoadStorePerTypeCppTemplateFileName;
	static const char* s_softClassLoadStorePerTypeCppTemplateFileName;
	static const char* s_softPtrLoadStoreCppFileName;

	static constexpr int k_numSoftObjectLoadStoreTypes = 3;
	static constexpr int k_numSoftClassLoadStoreTypes = 1;
	static const char* s_softObjectLoadStoreTypeNames[k_numSoftObjectLoadStoreTypes];
	static const char* s_softClassLoadStoreTypeNames[k_numSoftClassLoadStoreTypes];
	static const char* s_softPtrLoadStoreIncludes[k_numSoftObjectLoadStoreTypes + k_numSoftClassLoadStoreTypes];

	struct ParseTemplateParams
	{
		std::string m_templateFilePath = "";
		std::string m_perRecordTemplateFilePath = "";
		std::string m_perRecordEditorTemplateFilePath = "";
		std::string m_fileNameSuffix = "";
		std::string m_directorySuffix = "";
	};

	static bool ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseRecordDependentHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents, std::vector<std::string>& outHeaderFilePaths);
	static bool ParseRecordDependentCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParsePerRecordEditorTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParseSoftPtrLoadStoreTemplate(const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static bool ParseSoftPtrLoadStorePerTypeTemplate(const ParseTemplateParams& templateParams, bool generateSoftClasses, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
	static void PopulateDirectoryStrings(FString& staticDataDirectory, FString& utilitiesDirectory);
};