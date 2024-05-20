// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "TaskComponentData.generated.h"

UCLASS()
class ARGUS_API UTaskComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETask m_currentTask = ETask::None;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};