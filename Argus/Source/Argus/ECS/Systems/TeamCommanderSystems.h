// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class UAbilityRecord;

class TeamCommanderSystems
{
public:
	static void RunSystems(float deltaTime);
	static void InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent);
	static void PerformInitialUpdate();

	static int32 GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent);
	static FVector GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, const TeamCommanderComponent* teamCommanderComponent);
	static int32 GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent);
	static void ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate);
	static void ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex);

public:
#if !UE_BUILD_SHIPPING
	static void DebugRevealedAreasForTeamEntityId(uint16 teamEntityId);
#endif //  !UE_BUILD_SHIPPING
};
