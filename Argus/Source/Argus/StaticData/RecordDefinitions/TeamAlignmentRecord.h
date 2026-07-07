// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "ComponentDependencies/Teams.h"
#include "TeamAlignmentRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UTeamAlignmentRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "TeamA", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamAEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamA", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamAAllies;

	UPROPERTY(EditAnywhere, Category = "TeamB", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamBEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamB", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamBAllies;

	UPROPERTY(EditAnywhere, Category = "TeamC", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamCEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamC", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamCAllies;

	UPROPERTY(EditAnywhere, Category = "TeamD", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamDEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamD", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamDAllies;

	UPROPERTY(EditAnywhere, Category = "TeamE", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamEEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamE", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamEAllies;

	UPROPERTY(EditAnywhere, Category = "TeamF", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamFEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamF", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamFAllies;

	UPROPERTY(EditAnywhere, Category = "TeamG", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamGEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamG", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamGAllies;

	UPROPERTY(EditAnywhere, Category = "TeamH", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamHEnemies;

	UPROPERTY(EditAnywhere, Category = "TeamH", meta = (Bitmask, BitmaskEnum = "/Script/Argus.ETeam"))
	uint8 m_teamHAllies;

	uint8 GetEnemyMaskForTeam(ETeam team) const
	{
		switch (team)
		{
			case ETeam::TeamA:
				return m_teamAEnemies;
			case ETeam::TeamB:
				return m_teamBEnemies;
			case ETeam::TeamC:
				return m_teamCEnemies;
			case ETeam::TeamD:
				return m_teamDEnemies;
			case ETeam::TeamE:
				return m_teamEEnemies;
			case ETeam::TeamF:
				return m_teamFEnemies;
			case ETeam::TeamG:
				return m_teamGEnemies;
			case ETeam::TeamH:
				return m_teamHEnemies;
			default: 
				break;
		}

		return 0u;
	}

	uint8 GetAllyMaskForTeam(ETeam team) const
	{
		switch (team)
		{
		case ETeam::TeamA:
			return m_teamAAllies;
		case ETeam::TeamB:
			return m_teamBAllies;
		case ETeam::TeamC:
			return m_teamCAllies;
		case ETeam::TeamD:
			return m_teamDAllies;
		case ETeam::TeamE:
			return m_teamEAllies;
		case ETeam::TeamF:
			return m_teamFAllies;
		case ETeam::TeamG:
			return m_teamGAllies;
		case ETeam::TeamH:
			return m_teamHAllies;
		default:
			break;
		}

		return 0u;
	}
};