// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

class ComponentImplementationGenerator
{
public:
	static void GenerateComponentImplementationCode(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData);
	static const char* s_componentImplementationsTemplateDirectorySuffix;

private:
	static const char* s_componentImplementationsDirectorySuffix;
	static const char* s_dynamicAllocComponentImplementationsDirectorySuffix;
	static const char* s_componentImplementationCppTemplateFilename;
	static const char* s_dynamicAllocComponentImplementationCppTemplateFilename;
	static const char* s_perObservableTemplateFilename;
	static const char* s_sharedFunctionalityTemplateFilename;
	static const char* s_componentCppSuffix;

	enum UnderlyingType : uint8
	{
		None,
		Integer,
		Float,
		Vector,
		Vector2,
		TimerHandle,
		ResourceSet,
		EntityKDTree,
		ObstacleKDTree,
		EntityKDTreeOutput,
		ObstacleKDTreeOutput,
		SpawnEntityInfo,
		Bool,
		ControlGroup,
		TeamCommanderPriority,
		ResourceExtractionData,
		Bitmask,
		StaticData,
		Enum,
		Observers,
		ConstructionData
	};

	enum ContainerType : uint8
	{
		NoContainer,
		Array,
		Queue,
		Deque,
		ExponentialSmoother,
		SOSSmoother,
		Optional,
		BitArray,
		Map
	};

	struct TypeInfo
	{
		std::string m_cleanTypeName;
		std::vector<UnderlyingType> m_templateTypes;
		UnderlyingType m_underlyingType = UnderlyingType::None;
		ContainerType m_containerType = ContainerType::NoContainer;

		TypeInfo(const std::string& typeString, const std::string& macroString);

		UnderlyingType GetTemplateParameter(int index) const;
	private:
		UnderlyingType DetermineType(const std::string& typeString, const std::string& macroString, std::string& outCleanTypeName);
		void ExtractTemplateParameters(const std::string& typeString, std::vector<UnderlyingType>& outPopulatedTemplateParameters);
	};
	

	static bool ParseComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static bool ParseDynamicAllocComponentImplementationCppFileTemplateWithReplacements(const ArgusCodeGeneratorUtil::ParseComponentDataOutput& parsedComponentData, std::vector<ArgusCodeGeneratorUtil::FileWriteData>& outParsedFileContents);
	static void GeneratePerVariableResetText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	static void GeneratePerVariableSerializeText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	static void GeneratePerVariableImGuiText(const std::vector<ArgusCodeGeneratorUtil::ParsedVariableData>& parsedVariableData, std::vector<std::string>& outParsedVariableContents);
	
	static bool PopulateAtomicFormattingFunction(UnderlyingType variableType, const std::string& cleanType, const std::string& macro, TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& functionToPopulate, std::string& extraDataToPopulate);
	static void FormatImGuiArrayField(const std::string& variableName, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction);
	static void FormatImGuiQueueField(const std::string& variableName, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiDequeField(const std::string& variableName, const std::string& extraData, std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& elementFormattingFunction);
	static void FormatImGuiMapField(const std::string& variableName, const std::string& extraData, const std::string& secondExtraData,  std::vector<std::string>& outParsedVariableContents, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& element0FormattingFunction, const TFunction<void(const std::string&, const std::string&, const std::string&, std::vector<std::string>&)>& element1FormattingFunction);
	static void FormatImGuiFloatField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiIntField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiBitmaskField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiBoolField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiRecordField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiFVectorField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiFVector2DField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiEnumField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiTimerField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiResourceSetField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiConstructionDataField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiOptionalField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiSpawnEntityInfoField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiControlGroupField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiTeamCommanderPriorityField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
	static void FormatImGuiResourceSourceExtractionDataField(const std::string& variableName, const std::string& extraData, const std::string& prefix, std::vector<std::string>& outParsedVariableContents);
};