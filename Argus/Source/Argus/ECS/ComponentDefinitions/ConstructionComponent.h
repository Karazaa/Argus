// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

UENUM()
enum class EConstructionType : uint8
{
	Automatic,
	Manual
};

struct ConstructionComponent
{
	float m_requiredWorkSeconds = 0.0f;

	ARGUS_IGNORE()
	float m_currentWorkSeconds = 0.0f;

	ARGUS_IGNORE()
	uint32 m_constructionAbilityRecordId = 0u;

	EConstructionType m_constructionType = EConstructionType::Automatic;

	ARGUS_IGNORE()
	TimerHandle m_automaticConstructionTimerHandle;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %f)    \n(%s: %f)"),
				ARGUS_NAMEOF(ConstructionComponent),
				ARGUS_NAMEOF(m_requiredWorkSeconds),
				m_requiredWorkSeconds,
				ARGUS_NAMEOF(m_currentWorkSeconds),
				m_currentWorkSeconds
			)
		);
	}
};