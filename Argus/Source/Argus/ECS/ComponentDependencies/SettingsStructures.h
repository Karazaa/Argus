// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavAgentSelector.h"
#include "SettingsStructures.generated.h"

USTRUCT(BlueprintType)
struct FGroupSizeRadiusPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 m_groupSizeLowerBound = 0;

	UPROPERTY(EditDefaultsOnly)
	FNavAgentSelector m_navAgentSelection;
};