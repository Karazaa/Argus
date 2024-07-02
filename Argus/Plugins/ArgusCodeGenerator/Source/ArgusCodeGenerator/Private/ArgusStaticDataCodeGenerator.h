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

	struct ParseTemplateParams
	{
		std::string templateFilePath = "";
		std::string perRecordTemplateFilePath = "";
	};

	static bool ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents);
};