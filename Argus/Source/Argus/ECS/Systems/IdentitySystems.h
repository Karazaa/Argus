// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class IdentitySystems
{
public:
	static void RegisterEntityAsSeenByOther(ArgusEntity perceivedEntity, ArgusEntity perceiverEntity);
};
