// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ArgusEntity
{
public:
	ArgusEntity(uint32_t id);
	~ArgusEntity();
	uint32_t GetId();

private:
	uint32_t m_id;
};
