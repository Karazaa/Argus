// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "RecordReferences/MaterialRecordReference.h"

struct DecalSystemsSettingsComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_TRANSIENT
	FUMaterialRecordReference m_moveToLocationDecalMaterial;

	ARGUS_COMP_TRANSIENT
	FUMaterialRecordReference m_attackMoveToLocationDecalMaterial;

	static const DecalSystemsSettingsComponent* Get();
};
