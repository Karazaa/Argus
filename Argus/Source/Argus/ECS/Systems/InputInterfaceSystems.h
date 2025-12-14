// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

struct InputInterfaceComponent;

class InputInterfaceSystems
{
public:
	static void CheckAndHandleEntityDoubleClick(ArgusEntity entity);

private:
	static void AddAdjacentLikeEntitiesAsSelected(ArgusEntity entity, InputInterfaceComponent* inputInterfaceComponent);
	static void AddEntityIdAsSelected(uint16 entityId);
};
