// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class UAbilityRecord;

class TeamCommanderSystems
{
public:
	static void RunSystems(float deltaTime);
	static void InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent);

private:
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);

	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);

	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);
	static void UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);

	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToConstructResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);

	static int32 GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static FVector GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, TeamCommanderComponent* teamCommanderComponent);
	static int32 GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate);
	static void ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex);

	static const UAbilityRecord* GetConstructResourceSinkAbility(ArgusEntity entity, EAbilityIndex& abilityIndex);
	static bool DoesAbilityConstructResourceSink(const UAbilityRecord* abilityRecord);
	static void FindTargetLocForConstructResourceSink(ArgusEntity entity, const UAbilityRecord* abilityRecord, TeamCommanderComponent* teamCommanderComponent);
};
