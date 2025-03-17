// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ComponentObserversGenerator
{
public:
	static void GenerateComponentObserversCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);

private:
	static const char* s_componentObserversDirectorySuffix;
	static const char* s_componentObserversHeaderTemplateFilename;
	static const char* s_perObservableAbstractFuncTemplateFilename;
	static const char* s_perObservableOnChangedTemplateFilename;
	static const char* s_componentObserversHeaderSuffix;
	static const char* s_componentObserversTemplateDirectorySuffix;

	static bool ParseComponentObserversHeaderFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
};