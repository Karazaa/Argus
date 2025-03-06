// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "ObserversComponentData.generated.h"

UCLASS()
class ARGUS_API UObserversComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	TaskComponentObservers m_taskComponentObservers;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
