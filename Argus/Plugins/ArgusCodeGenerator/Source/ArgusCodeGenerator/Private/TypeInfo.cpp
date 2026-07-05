// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TypeInfo.h"

TypeInfo::TypeInfo(const ArgusCodeGeneratorUtil::ParsedVariableData& variableData)
{
	m_cleanVariableName = variableData.m_varName;
	m_underlyingType = DetermineType(variableData.m_typeName, variableData.m_propertyMacro, m_cleanTypeName);

	if (variableData.m_typeName.find("ExponentialDecaySmoother") != std::string::npos)
	{
		m_containerType = ContainerType::ExponentialSmoother;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("SecondOrderSystemSmoother") != std::string::npos)
	{
		m_containerType = ContainerType::SOSSmoother;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("TOptional") != std::string::npos)
	{
		m_containerType = ContainerType::Optional;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("TArray") != std::string::npos)
	{
		m_containerType = ContainerType::Array;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("ArgusQueue") != std::string::npos)
	{
		m_containerType = ContainerType::Queue;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("ArgusDeque") != std::string::npos)
	{
		m_containerType = ContainerType::Deque;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (variableData.m_typeName.find("TBitArray") != std::string::npos)
	{
		m_containerType = ContainerType::BitArray;
	}
	else if (variableData.m_typeName.find("ArgusMap") != std::string::npos)
	{
		m_containerType = ContainerType::Map;
		ExtractTemplateParameters(variableData.m_typeName, m_templateTypes);
	}
	else if (!variableData.m_sizeString.empty())
	{
		m_containerType = ContainerType::CArray;
		m_staticSize = variableData.m_sizeString;
	}
}

UnderlyingType TypeInfo::GetTemplateParameter(int index) const
{
	if (index >= 0 && index < m_templateTypes.size())
	{
		return m_templateTypes[index];
	}

	return UnderlyingType::None;
}

UnderlyingType TypeInfo::DetermineType(const std::string& typeString, const std::string& macroString, std::string& outCleanTypeName)
{
	if (typeString.at(0) == '\t')
	{
		outCleanTypeName = typeString.substr(1, typeString.length() - 1);
	}
	else
	{
		outCleanTypeName = typeString;
	}
	UnderlyingType output = UnderlyingType::None;

	// BRUH REALLY? Yes. Until I find a better way to differentiate enum types... here we are.
	if (outCleanTypeName.at(0) == 'E')
	{
		output = UnderlyingType::Enum;
	}
	else if (typeString.find("BITMASK") != std::string::npos)
	{
		output = UnderlyingType::Bitmask;
	}
	else if (macroString.find(ArgusCodeGeneratorUtil::s_propertyStaticDataDelimiter) != std::string::npos)
	{
		output = UnderlyingType::StaticData;
	}
	else if (typeString.find("uint8") != std::string::npos ||
		typeString.find("uint16") != std::string::npos ||
		typeString.find("uint32") != std::string::npos ||
		typeString.find("int8") != std::string::npos ||
		typeString.find("int16") != std::string::npos ||
		typeString.find("int32") != std::string::npos)
	{
		output = UnderlyingType::Integer;
	}
	else if (typeString.find("float") != std::string::npos)
	{
		output = UnderlyingType::Float;
	}
	else if (typeString.find("FVector2D") != std::string::npos)
	{
		output = UnderlyingType::Vector2;
	}
	else if (typeString.find("FVector") != std::string::npos)
	{
		output = UnderlyingType::Vector;
	}
	else if (typeString.find("TimerHandle") != std::string::npos)
	{
		output = UnderlyingType::TimerHandle;
	}
	else if (typeString.find("FResourceSet") != std::string::npos)
	{
		output = UnderlyingType::ResourceSet;
	}
	else if (typeString.find("ArgusEntityKDTreeRangeOutput") != std::string::npos)
	{
		output = UnderlyingType::EntityKDTreeOutput;
	}
	else if (typeString.find("ObstaclePointKDTreeRangeOutput") != std::string::npos)
	{
		output = UnderlyingType::ObstacleKDTreeOutput;
	}
	else if (typeString.find("ArgusEntityKDTree") != std::string::npos)
	{
		output = UnderlyingType::EntityKDTree;
	}
	else if (typeString.find("ObstaclePointKDTree") != std::string::npos)
	{
		output = UnderlyingType::ObstacleKDTree;
	}
	else if (typeString.find("SpawnEntityInfo") != std::string::npos)
	{
		output = UnderlyingType::SpawnEntityInfo;
	}
	else if (typeString.find("bool") != std::string::npos)
	{
		output = UnderlyingType::Bool;
	}
	else if (typeString.find("ControlGroup") != std::string::npos)
	{
		output = UnderlyingType::ControlGroup;
	}
	else if (typeString.find("TeamCommanderPriority") != std::string::npos)
	{
		output = UnderlyingType::TeamCommanderPriority;
	}
	else if (typeString.find("ResourceSourceExtractionData") != std::string::npos)
	{
		output = UnderlyingType::ResourceExtractionData;
	}
	else if (typeString.find("Observers") != std::string::npos)
	{
		output = UnderlyingType::Observers;
	}
	else if (typeString.find("ConstructionData") != std::string::npos)
	{
		output = UnderlyingType::ConstructionData;
	}
	else if (typeString.find("FNavAgentSelector") != std::string::npos)
	{
		output = UnderlyingType::NavAgentSelector;
	}

	return output;
}

void TypeInfo::ExtractTemplateParameters(const std::string& typeString, std::vector<UnderlyingType>& outPopulatedTemplateParameters)
{
	const size_t indexOfTemplateParamStart = typeString.find_first_of('<');
	const size_t indexOfTemplateParamEnd = typeString.find_first_of('>');

	std::string templateParamString = typeString.substr(indexOfTemplateParamStart + 1, indexOfTemplateParamEnd - indexOfTemplateParamStart);

	size_t currentParseIndex = 0;
	while (currentParseIndex != std::string::npos)
	{
		size_t previousParseIndex = currentParseIndex;
		currentParseIndex = templateParamString.find(',', currentParseIndex + 1);
		if (currentParseIndex != std::string::npos)
		{
			currentParseIndex++;
		}

		std::string underlyingTypeString = templateParamString.substr(previousParseIndex, currentParseIndex - (previousParseIndex + (currentParseIndex == std::string::npos ? 0 : 1)));
		if (underlyingTypeString.find("ArgusContainerAllocator") == std::string::npos && underlyingTypeString.find("ArgusSetAllocator") == std::string::npos)
		{
			std::string unusedCleanTypeName;
			outPopulatedTemplateParameters.push_back(DetermineType(underlyingTypeString, "", unusedCleanTypeName));
		}
	}
}