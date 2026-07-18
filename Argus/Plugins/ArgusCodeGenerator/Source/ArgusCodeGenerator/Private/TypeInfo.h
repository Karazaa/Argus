// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCodeGeneratorUtil.h"

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
	ConstructionData,
	NavAgentSelector
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
	Map,
	CArray,
	Set
};

struct TypeInfo
{
	std::string m_cleanTypeName;
	std::string m_cleanVariableName;
	std::string m_staticSize;
	std::vector<UnderlyingType> m_templateTypes;
	UnderlyingType m_underlyingType = UnderlyingType::None;
	ContainerType m_containerType = ContainerType::NoContainer;

	TypeInfo(const ArgusCodeGeneratorUtil::ParsedVariableData& variableData);

	UnderlyingType GetTemplateParameter(int index) const;
	bool HasTemplateParameters() const;

private:
	UnderlyingType DetermineType(const std::string& typeString, const std::string& macroString, std::string& outCleanTypeName);
	void ExtractTemplateParameters(const std::string& typeString, std::vector<UnderlyingType>& outPopulatedTemplateParameters);
};