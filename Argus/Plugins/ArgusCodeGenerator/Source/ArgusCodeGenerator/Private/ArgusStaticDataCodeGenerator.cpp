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
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseHeaderTemplateFileName = "ArgusStaticDatabaseHeaderTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseHeaderPerRecordTemplateFileName = "ArgusStaticDatabaseHeaderPerRecordTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseHeaderFileName = "ArgusStaticDatabase.h";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseCppTemplateFileName = "ArgusStaticDatabaseCppTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseCppPerRecordTemplateFileName = "ArgusStaticDatabaseCppPerRecordTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDatabaseCppFileName = "ArgusStaticDatabase.cpp";
const char* ArgusStaticDataCodeGenerator::s_argusStaticDataFileName = "ArgusStaticData.h";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseDirectorySuffix = "RecordDatabases/";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseHeaderTemplateFileName = "RecordDatabaseHeaderTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseCppTemplateFileName = "RecordDatabaseCppTemplate.txt";
const char* ArgusStaticDataCodeGenerator::s_recordDatabaseFileNameSuffix = "Database";

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

	ParseTemplateParams parseArgusStaticDatabaseHeaderTemplateParams;
	parseArgusStaticDatabaseHeaderTemplateParams.templateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDatabaseHeaderTemplateFileName);
	parseArgusStaticDatabaseHeaderTemplateParams.perRecordTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDatabaseHeaderPerRecordTemplateFileName);

	ParseTemplateParams parseArgusStaticDatabaseCppTemplateParams;
	parseArgusStaticDatabaseCppTemplateParams.templateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDatabaseCppTemplateFileName);
	parseArgusStaticDatabaseCppTemplateParams.perRecordTemplateFilePath = std::string(cStrTemplateDirectory).append(s_argusStaticDatabaseCppPerRecordTemplateFileName);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate non-ECS Static Data code."), ARGUS_FUNCNAME)

	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedArgusStaticDataContents = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	didSucceed &= ParseArgusStaticDataTemplate(parsedStaticDataRecords, parseArgusStaticDataTemplateParams, outParsedArgusStaticDataContents);
	std::vector<std::string> headerFilePaths;
	didSucceed &= ParseRecordDatabaseHeaderTemplate(parsedStaticDataRecords, parseRecordDatabaseHeaderTemplateParams, outParsedArgusStaticDataContents, headerFilePaths);
	didSucceed &= ParseRecordDatabaseCppTemplate(parsedStaticDataRecords, parseRecordDatabaseCppTemplateParams, headerFilePaths, outParsedArgusStaticDataContents);
	didSucceed &= ParseArgusStaticDatabaseHeaderTemplate(parsedStaticDataRecords, parseArgusStaticDatabaseHeaderTemplateParams, headerFilePaths, outParsedArgusStaticDataContents);
	didSucceed &= ParseArgusStaticDatabaseCppTemplate(parsedStaticDataRecords, parseArgusStaticDatabaseCppTemplateParams, headerFilePaths, outParsedArgusStaticDataContents);

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

bool ArgusStaticDataCodeGenerator::ParseArgusStaticDataTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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

bool ArgusStaticDataCodeGenerator::ParseRecordDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents, std::vector<std::string>& outHeaderFilePaths)
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
		std::string fileName = parsedStaticDataRecords.m_staticDataRecordNames[i];
		fileName = fileName.substr(1, fileName.length() - 1);
		fileName.append(s_recordDatabaseFileNameSuffix);

		for (int j = 0; j < fileContents.size(); ++j)
		{
			if (fileContents[j].find("$$$$$") != std::string::npos)
			{
				writeData.m_lines.push_back(parsedStaticDataRecords.m_staticDataIncludeStatements[i]);
			}
			else if (fileContents[j].find("%%%%%") != std::string::npos)
			{
				writeData.m_lines.push_back(std::regex_replace(fileContents[j], std::regex("%%%%%"), fileName));
			}
			else if (fileContents[j].find("#####") != std::string::npos)
			{
				writeData.m_lines.push_back(std::regex_replace(fileContents[j], std::regex("#####"), parsedStaticDataRecords.m_staticDataRecordNames[i]));
			}
			else
			{
				writeData.m_lines.push_back(fileContents[j]);
			}
		}

		std::string filePath = s_recordDatabaseDirectorySuffix;
		filePath.append(fileName);
		outHeaderFilePaths.push_back(filePath);
		filePath.append(".h");
		writeData.m_filename = filePath;
		outParsedFileContents.push_back(writeData);
	}

	return true;
}

bool ArgusStaticDataCodeGenerator::ParseRecordDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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

		for (int j = 0; j < fileContents.size(); ++j)
		{
			if (fileContents[j].find("$$$$$") != std::string::npos)
			{
				std::string includeStatement = "#include \"";
				std::string filePath = headerFilePaths[i];
				filePath.append(".h");
				includeStatement.append(filePath).append("\"");
				writeData.m_lines.push_back(includeStatement);
			}
			else if (fileContents[j].find("#####") != std::string::npos)
			{
				writeData.m_lines.push_back(std::regex_replace(fileContents[j], std::regex("#####"), parsedStaticDataRecords.m_staticDataRecordNames[i]));
			}
			else
			{
				writeData.m_lines.push_back(fileContents[j]);
			}
		}

		writeData.m_filename = headerFilePaths[i];
		writeData.m_filename.append(".cpp");
		outParsedFileContents.push_back(writeData);
	}

	return true;
}

bool ArgusStaticDataCodeGenerator::ParseArgusStaticDatabaseHeaderTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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

	ArgusCodeGeneratorUtil::FileWriteData writeData;
	writeData.m_filename = s_argusStaticDatabaseHeaderFileName;
	for (int i = 0; i < fileContents.size(); ++i)
	{
		if (fileContents[i].find("@@@@@") != std::string::npos)
		{
			ParsePerRecordTemplate(parsedStaticDataRecords, templateParams, writeData);
		}
		else if (fileContents[i].find("$$$$$") != std::string::npos)
		{
			for (int j = 0; j < headerFilePaths.size(); ++j)
			{
				std::string includeStatement = "#include \"";
				std::string filePath = headerFilePaths[j];
				filePath.append(".h");
				includeStatement.append(filePath).append("\"");
				writeData.m_lines.push_back(includeStatement);
			}
		}
		else
		{
			writeData.m_lines.push_back((fileContents[i]));
		}
	}

	outParsedFileContents.push_back(writeData);
	return true;
}

bool ArgusStaticDataCodeGenerator::ParseArgusStaticDatabaseCppTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, const std::vector<std::string>& headerFilePaths, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
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

	ArgusCodeGeneratorUtil::FileWriteData writeData;
	writeData.m_filename = s_argusStaticDatabaseCppFileName;
	for (int i = 0; i < fileContents.size(); ++i)
	{
		if (fileContents[i].find("@@@@@") != std::string::npos)
		{
			ParsePerRecordTemplate(parsedStaticDataRecords, templateParams, writeData);
		}
		else
		{
			writeData.m_lines.push_back((fileContents[i]));
		}
	}

	outParsedFileContents.push_back(writeData);
	return true;
}

bool ArgusStaticDataCodeGenerator::ParsePerRecordTemplate(const ArgusCodeGeneratorUtil::ParseStaticDataRecordsOutput& parsedStaticDataRecords, const ParseTemplateParams& templateParams, ArgusCodeGeneratorUtil::FileWriteData& outParsedFileContents)
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