// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticDataCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ArgusStaticDataCodeGenerator::s_staticDataTemplateDirectorySuffix = "StaticData/";
const char* ArgusStaticDataCodeGenerator::s_staticDataDirectorySuffix = "Source/Argus/StaticData/";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDataTemplateFileName = "ArgusStaticDataTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDataPerRecordTemplateFileName = "ArgusStaticDataPerRecordTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDataFileName = "ArgusStaticData.h";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseDirectorySuffix = "RecordDatabases/";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseHeaderTemplateFileName = "RecordDatabaseHeaderTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseCppTemplateFileName = "RecordDatabaseCppTemplate.txt";

void ArgusStaticDataCodeGenerator::GenerateStaticDataCode(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of Argus non-ECS Static Data code."), ARGUS_FUNCNAME)
	bool didSucceed = true;

	// Construct a directory path to static data templates
	const char* cStrTemplateDirectory = TCHAR_TO_UTF8(*ArgusCodeGeneratorUtil::GetTemplateDirectory(s_staticDataTemplateDirectorySuffix));

	ParseTemplateParams parseArgusStaticDataTemplateParams;
	parseArgusStaticDataTemplateParams.templateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDataTemplateFileName);
	parseArgusStaticDataTemplateParams.perRecordTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDataPerRecordTemplateFileName);

	ParseTemplateParams parseRecordDatabaseHeaderTemplateParams;
	parseRecordDatabaseHeaderTemplateParams.templateFilePath = std::string(cStrTemplateDirectory).append(s_recordDatabaseHeaderTemplateFileName);

	ParseTemplateParams parseRecordDatabaseCppTemplateParams;
	parseRecordDatabaseCppTemplateParams.templateFilePath = std::string(cStrTemplateDirectory).append(s_recordDatabaseCppTemplateFileName);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate non-ECS Static Data code."), ARGUS_FUNCNAME)

	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedArgusStaticDataContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseArgusStaticDataTemplate(parsedStaticDataRecords, parseArgusStaticDataTemplateParams, outParsedArgusStaticDataContents);
	didSucceed &= ParseRecordDatabaseHeaderTemplate(parsedStaticDataRecords, parseRecordDatabaseHeaderTemplateParams, outParsedArgusStaticDataContents);
	didSucceed &= ParseRecordDatabaseCppTemplate(parsedStaticDataRecords, parseRecordDatabaseCppTemplateParams, outParsedArgusStaticDataContents);

	FString staticDataDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	staticDataDirectory.Append(s_staticDataDirectorySuffix);
	FPaths::MakeStandardFilename(staticDataDirectory);
	const char* cStrStaticDataDirectory = TCHAR_TO_UTF8(*staticDataDirectory);

	// Write out header and cpp files.
	for (int i = 0; i < outParsedArgusStaticDataContents.size(); ++i)
	{
		didSucceed &= ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrStaticDataDirectory).append(outParsedArgusStaticDataContents[i].m_filename), outParsedArgusStaticDataContents[i].m_lines);
	}
}

bool ArgusStaticDataCodeGenerator::ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateParams.templateFilePath);
	const FString ueTemplateFilePath = FString(templateParams.templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	ArgusCodeGeneratorUtil::FileWriteData writeData;
	writeData.m_filename = s_argusStaticDataFileName;
	outParsedFileContents.push_back(writeData);

	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		if (templateLineText.find("@@@@@") != std::string::npos)
		{
			ParsePerRecordTemplate(parsedStaticDataRecords, templateParams, outParsedFileContents.back());
		}
		else
		{
			outParsedFileContents.back().m_lines.push_back(templateLineText);
		}
	}

	inTemplateStream.close();
	return true;
}

bool ArgusStaticDataCodeGenerator::ParseRecordDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateParams.templateFilePath);
	const FString ueTemplateFilePath = FString(templateParams.templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	std::vector<std::string> fileContents;
	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		fileContents.push_back(templateLineText);
	}
	inTemplateStream.close();

	for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		// TODO JAMES: Construct output file name.

		for (int j = 0; j < fileContents.size(); ++j)
		{
			if (fileContents[j].find("$$$$$") != std::string::npos)
			{
				// TODO JAMES: Write include statement from parsed records
			}
			else if (fileContents[j].find("%%%%%") != std::string::npos)
			{
				// TODO JAMES: Do gross string splicing to output generated file name
			}
			else if (fileContents[j].find("#####") != std::string::npos)
			{
				// TODO JAMES: Direct name replacement.
			}
			else
			{
				writeData.m_lines.push_back(fileContents[j]);
			}
		}

		// TODO JAMES: uncomment: outParsedFileContents.push_back(writeData);
	}

	return true;
}

bool ArgusStaticDataCodeGenerator::ParseRecordDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateParams.templateFilePath);
	const FString ueTemplateFilePath = FString(templateParams.templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	std::vector<std::string> fileContents;
	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		fileContents.push_back(templateLineText);
	}
	inTemplateStream.close();

	for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
	{
		ArgusCodeGeneratorUtil::FileWriteData writeData;
		// TODO JAMES: Construct output file name.

		for (int j = 0; j < fileContents.size(); ++j)
		{
			if (fileContents[j].find("$$$$$") != std::string::npos)
			{
				// TODO JAMES: Write include statement from parsed records
			}
			else if (fileContents[j].find("#####") != std::string::npos)
			{
				// TODO JAMES: Direct name replacement.
			}
			else
			{
				writeData.m_lines.push_back(fileContents[j]);
			}
		}

		// TODO JAMES: uncomment: outParsedFileContents.push_back(writeData);
	}

	return true;
}

bool ArgusStaticDataCodeGenerator::ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents)
{
	std::ifstream inPerRecordTemplateStream = std::ifstream(templateParams.perRecordTemplateFilePath);
	const FString uePerRecordTemplateFilePath = FString(templateParams.perRecordTemplateFilePath.c_str());
	if (!inPerRecordTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *uePerRecordTemplateFilePath);
		return false;
	}

	std::vector<std::string> fileContents;

	std::string templateLineText;
	while (std::getline(inPerRecordTemplateStream, templateLineText))
	{
		fileContents.push_back(templateLineText);
	}
	inPerRecordTemplateStream.close();

	for (int i = 0; i < parsedStaticDataRecords.m_staticDataRecordNames.size(); ++i)
	{
		for (int j = 0; j < fileContents.size(); ++j)
		{
			if (fileContents[j].find("#####") != std::string::npos)
			{
				std::string perComponentHeaderLineText = fileContents[j];
				outParsedFileContents.m_lines.push_back(std::regex_replace(perComponentHeaderLineText, std::regex("#####"), parsedStaticDataRecords.m_staticDataRecordNames[i]));
			}
			else
			{
				outParsedFileContents.m_lines.push_back(fileContents[j]);
			}
		}
	}

	return true;
}