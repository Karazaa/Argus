// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "RecordReferences/MaterialRecordReference.h"

struct DecalSystemsSettingsComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	FUMaterialRecordReference m_moveToLocationDecalMaterial;
	FUMaterialRecordReference m_attackMoveToLocationDecalMaterial;

	static const DecalSystemsSettingsComponent* Get();
};
