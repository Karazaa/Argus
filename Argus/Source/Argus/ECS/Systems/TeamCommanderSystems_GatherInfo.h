// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_GatherInfo
{
public:
	static void RunSystems();

private:
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkEntity, ResourceSourceExtractionData& data);
	static void ClearResourceSinkConstructorFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkConstructorEntity, ResourceSourceExtractionData& data);
	static void ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity existingResourceExtractorEntity, ResourceSourceExtractionData& data);

	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateSpawningUnitTypesPerSpawner(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
};
