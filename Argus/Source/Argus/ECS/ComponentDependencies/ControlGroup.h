// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

struct ControlGroup
{
	TArray<uint16> m_entityIds;

	int32 Max() { return m_entityIds.Max(); }
	void Reserve(int32 size) { m_entityIds.Reserve(size); }
	void Reset() { m_entityIds.Reset(); }
	void Add(uint16 entityId) { m_entityIds.Add(entityId); }
};