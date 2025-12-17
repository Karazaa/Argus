// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

struct ControlGroup
{
	TArray<uint16> m_entityIds;

	int32 Max() const { return m_entityIds.Max(); }
	int32 Num() const { return m_entityIds.Num(); }
	void Reserve(int32 size) { m_entityIds.Reserve(size); }
	void Reset() { m_entityIds.Reset(); }
	void Add(uint16 entityId) { m_entityIds.Add(entityId); }
	uint16& operator[](int32 index) { return m_entityIds[index]; }
	const uint16& operator[](int32 index) const { return m_entityIds[index]; }
};