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
	static const char* s_argusStaticDataTemplateFileName;
	static const char* s_argusStaticDataPerRecordTemplateFileName;
	static const char* s_argusStaticDataFileName;
	static const char* s_argusStaticDatabaseHeaderTemplateFileName;
	static const char* s_argusStaticDatabaseHeaderPerRecordTemplateFileName;
	static const char* s_argusStaticDatabaseCppTemplateFileName;
	static const char* s_argusStaticDatabaseCppPerRecordTemplateFileName;
	static const char* s_recordDatabaseDirectorySuffix;
	static const char* s_recordDatabaseHeaderTemplateFileName;
	static const char* s_recordDatabaseCppTemplateFileName;
	static const char* s_recordDatabaseFileNameSuffix;

	struct ParseTemplateParams
	{
		std::string templateFilePath = "";
		std::string perRecordTemplateFilePath = "";
	};

	static bool ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseRecordDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents, std::vector<std::string>& outHeaderFilePaths);
	static bool ParseRecordDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseArgusStaticDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
};