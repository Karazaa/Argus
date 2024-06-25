// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticDataCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ArgusStaticDataCodeGenerator::s_staticDataTemplateDirectorySuffix = "StaticData/";

void ArgusStaticDataCodeGenerator::GenerateStaticDataCode(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus non-ECS Static Data code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Construct a directory path to static data templates
	const char* cStrTemplateDirectory = TCHAR_TO_UTF8(*ArgusCodeGeneratorUtil::GetTemplateDirectory(s_staticDataTemplateDirectorySuffix));
}