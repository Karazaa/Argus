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

private:
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkEntity, ResourceSourceExtractionData& data);
	static void ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity existingResourceExtractorEntity, ResourceSourceExtractionData& data);

	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);

	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);
	static void UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);

	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToConstructResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignEntityToSpawnUnitIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);

	static int32 GetAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent);
	static FVector GetWorldSpaceLocationFromAreaIndex(int32 areaIndex, const TeamCommanderComponent* teamCommanderComponent);
	static int32 GetClosestUnrevealedAreaToEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponent* teamCommanderComponent);
	static void ConvertAreaIndexToAreaCoordinates(int32 areaIndex, int32 areasPerDimension, int32& xCoordinate, int32& yCoordinate);
	static void ConvertAreaCoordinatesToAreaIndex(int32 xCoordinate, int32 yCoordinate, int32 areasPerDimension, int32& areaIndex);

	static bool GetConstructResourceSinkAbilities(ArgusEntity entity, EResourceType type, TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& outAbilityIndexPairs);
	static bool GetSpawnUnitAbilities(ArgusEntity entity, ESpawnUnitType unitType, EResourceType resourceType, TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& outAbilityIndexPairs);
	static bool DoesAbilityConstructResourceSink(const UAbilityRecord* abilityRecord, EResourceType type);
	static bool DoesAbilitySpawnUnitType(const UAbilityRecord* abilityRecord, ESpawnUnitType unitType, EResourceType resourceType = EResourceType::Count);
	static bool FindTargetLocForConstructResourceSink(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type);
	static ArgusEntity GetNearestSeenResourceSourceToEntity(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderComponent* teamCommanderComponent, EResourceType type, int32& outPairIndex);

public:
#if !UE_BUILD_SHIPPING
	static void DebugRevealedAreasForTeamEntityId(uint16 teamEntityId);
#endif //  !UE_BUILD_SHIPPING
};
