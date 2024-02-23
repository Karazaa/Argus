// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusComponentRegistry.h"

HealthComponent ArgusComponentRegistry::m_healthComponentExample = HealthComponent();

template<>
HealthComponent* ArgusComponentRegistry::GetComponent(uint16 entityId)
{
	return &m_healthComponentExample;
}