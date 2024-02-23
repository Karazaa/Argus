// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentDefinitions.h"
#include "CoreMinimal.h"
#include <optional>

class ArgusComponentRegistry
{
public:
	template<class ArgusComponent>
	static ArgusComponent* GetComponent(uint16 entityId)
	{
		return nullptr;
	}
private:

	// TODO JAMES: Clean up exaple health component used for testing.
	static HealthComponent m_healthComponentExample;
};