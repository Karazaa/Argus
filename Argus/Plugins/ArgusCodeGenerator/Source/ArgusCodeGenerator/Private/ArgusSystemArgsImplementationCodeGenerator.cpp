// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemArgsImplementationCodeGenerator.h"
#include "Misc/Paths.h"
#include <filesystem>
#include <fstream>
#include <regex>

const char* ArgusSystemArgsImplementationCodeGenerator::s_systemArgTemplateDirectorySuffix = "SystemArguments/";
const char* ArgusSystemArgsImplementationCodeGenerator::s_systemArgImplementationTemplateFileName = "SystemArgumentImplementationTemplate.txt";

void ArgusSystemArgsImplementationCodeGenerator::GenerateSystemArgsImplementation(const ArgusCodeGeneratorUtil::ParseSystemArgDefinitionsOutput& parsedSystemArgs)
{
	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Starting generation of system argument implementations for Argus Systems."), ARGUS_FUNCNAME);
	bool didSucceed = true;

	// Construct a directory path to static data templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(s_systemArgTemplateDirectorySuffix));
	std::string templateFilePath = std::string(cStrTemplateDirectory).append(s_systemArgImplementationTemplateFileName);

	UE_LOG(ArgusCodeGeneratorLog, Display, TEXT("[%s] Parsing from template files to generate non-ECS Static Data code."), ARGUS_FUNCNAME);
	std::vector<ArgusCodeGeneratorUtil::FileWriteData> outParsedSystemArgImplementations = std::vector<ArgusCodeGeneratorUtil::FileWriteData>();
	ParseSystemArgumentImplementationTemplate(parsedSystemArgs, templateFilePath, outParsedSystemArgImplementations);
}

bool ArgusSystemArgsImplementationCodeGenerator::ParseSystemArgumentImplementationTemplate(const ArgusCodeGeneratorUtil::ParseSystemArgDefinitionsOutput& parsedSystemArgs, const std::string& templateFilePath, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents)
{
	std::ifstream inTemplateStream = std::ifstream(templateFilePath);
	const FString ueTemplateFilePath = FString(templateFilePath.c_str());
	if (!inTemplateStream.is_open())
	{
		UE_LOG(ArgusCodeGeneratorLog, Error, TEXT("[%s] Failed to read from template file: %s"), ARGUS_FUNCNAME, *ueTemplateFilePath);
		return false;
	}

	for (int i = 0; i < parsedSystemArgs.m_systemArgsNames.size(); ++i)
	{
		outParsedFileContents.push_back(ArgusCodeGeneratorUtil::FileWriteData());
	}

	std::string templateLineText;
	while (std::getline(inTemplateStream, templateLineText))
	{
		if (templateLineText.find("&&&&&") != std::string::npos)
		{
			for (int i = 0; i < parsedSystemArgs.m_systemArgsIncludeStatements.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(parsedSystemArgs.m_systemArgsIncludeStatements[i]);
			}
		}
		else if (templateLineText.find("$$$$$") != std::string::npos)
		{
			for (int i = 0; i < parsedSystemArgs.m_systemArgsNames.size(); ++i)
			{
				for (int j = 1; j < parsedSystemArgs.m_systemArgsVariableData[i].size(); ++j)
				{
					std::string line = parsedSystemArgs.m_systemArgsVariableData[i][j].m_varName;
					std::string typeName = parsedSystemArgs.m_systemArgsVariableData[i][j].m_typeName;
					TrimTypeName(typeName);
					outParsedFileContents[i].m_lines.push_back(std::vformat("\t{} = entity.GetComponent<{}>();", std::make_format_args(parsedSystemArgs.m_systemArgsVariableData[i][j].m_varName, typeName)));
				}
			}
		}
		else if (templateLineText.find("%%%%%") != std::string::npos)
		{
			for (int i = 0; i < parsedSystemArgs.m_systemArgsNames.size(); ++i)
			{
				for (int j = 0; j < parsedSystemArgs.m_systemArgsVariableData[i].size(); ++j)
				{

				}
			}
		}
		else if (templateLineText.find("#####") != std::string::npos)
		{
			for (int i = 0; i < parsedSystemArgs.m_systemArgsNames.size(); ++i)
			{
				std::string perArgLineText = templateLineText;
				outParsedFileContents[i].m_lines.push_back(std::regex_replace(perArgLineText, std::regex("#####"), parsedSystemArgs.m_systemArgsNames[i]));
			}
		}
		else
		{
			for (int i = 0; i < parsedSystemArgs.m_systemArgsNames.size(); ++i)
			{
				outParsedFileContents[i].m_lines.push_back(templateLineText);
			}
		}
	}

	inTemplateStream.close();
	return true;
}

void ArgusSystemArgsImplementationCodeGenerator::TrimTypeName(std::string& typeName)
{
	int start = 1;
	if (typeName.starts_with("\tconst"))
	{
		start = 7;
	}

	int offset = start;
	if (typeName.ends_with('*'))
	{
		offset++;
	}

	typeName = typeName.substr(start, typeName.size() - offset);
}
