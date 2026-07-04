// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderComponentCollection.h"
#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_GatherInfo
{
public:
	static void RunSystems();

private:
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkEntity, ResourceSourceExtractionData& data);
	static void ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity existingResourceExtractorEntity, ResourceSourceExtractionData& data);

	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateSeenByTeamCommandersPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateSeenByTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents);
	static void UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents);
	static void UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents);
	static void UpdateSpawningUnitTypesPerSpawner(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents);
	static void UpdateConstructionDataPerConstructee(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents);
};
