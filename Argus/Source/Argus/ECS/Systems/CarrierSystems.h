// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class CarrierSystems
{
public:
	static bool CanEntityCarryOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity);
};
