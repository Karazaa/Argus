// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <optional>

class ArgusComponentRegistry
{
public:
	template<class Component>
	static std::optional<Component> GetComponent(uint16 entityId);
};