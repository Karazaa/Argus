// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct WorldCellComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA
	int32 m_cellXCoordinate = 0;

	ARGUS_COMP_NO_DATA
	int32 m_cellYCoordinate = 0;
};
