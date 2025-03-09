// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "PlacedArgusActorTeamInfoRecord.generated.h"

class AArgusActor;

UCLASS(BlueprintType)
class ARGUS_API UPlacedArgusActorTeamInfoRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETeam m_team = ETeam::None;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = ETeam))
	uint8 m_allies = 0u;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = ETeam))
	uint8 m_enemies = 0u;
};