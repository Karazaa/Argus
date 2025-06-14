// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "InputInterfaceComponentData.generated.h"

UCLASS()
class ARGUS_API UInputInterfaceComponentData : public UComponentData
{
	GENERATED_BODY()

public:

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};
