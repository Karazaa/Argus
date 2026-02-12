// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

class UAbilityRecord;

UENUM(BlueprintType)
enum class EAbilityIndex : uint8
{
	None = 0,
	Ability0 = 1 << 0,
	Ability1 = 1 << 1,
	Ability2 = 1 << 2,
	Ability3 = 1 << 3
};

struct AbilityComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability0Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability1Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability2Id = 0u;

	ARGUS_STATIC_DATA(UAbilityRecord)
	uint32 m_ability3Id = 0u;

	ARGUS_IGNORE()
	uint32 m_ability0OverrideId = 0u;

	ARGUS_IGNORE()
	uint32 m_ability1OverrideId = 0u;

	ARGUS_IGNORE()
	uint32 m_ability2OverrideId = 0u;

	ARGUS_IGNORE()
	uint32 m_ability3OverrideId = 0u;

	ARGUS_IGNORE()
	uint32 m_abilityToRefundId = 0u;

	ARGUS_OBSERVABLE_DECLARATION(uint8, m_abilityOverrideBitmask, 0u)

	uint8 m_abilityCasterPriority = 0u;

	uint32 GetActiveAbilityId(EAbilityIndex abilityIndex) const
	{
		if (m_abilityOverrideBitmask & static_cast<uint8>(abilityIndex))
		{
			switch (abilityIndex)
			{
				case EAbilityIndex::Ability0:
					return m_ability0OverrideId;

				case EAbilityIndex::Ability1:
					return m_ability1OverrideId;

				case EAbilityIndex::Ability2:
					return m_ability2OverrideId;

				case EAbilityIndex::Ability3:
					return m_ability3OverrideId;

				default:
					break;
			}
		}

		switch (abilityIndex)
		{
			case EAbilityIndex::Ability0:
				return m_ability0Id;

			case EAbilityIndex::Ability1:
				return m_ability1Id;

			case EAbilityIndex::Ability2:
				return m_ability2Id;

			case EAbilityIndex::Ability3:
				return m_ability3Id;

			default:
				break;
		}

		return 0u;
	}

	void AddAbilityOverride(uint32 abilityOverrideId, EAbilityIndex abilityIndex)
	{
		switch (abilityIndex)
		{
			case EAbilityIndex::Ability0:
				m_ability0OverrideId = abilityOverrideId;
				break;

			case EAbilityIndex::Ability1:
				m_ability1OverrideId = abilityOverrideId;
				break;

			case EAbilityIndex::Ability2:
				m_ability2OverrideId = abilityOverrideId;
				break;

			case EAbilityIndex::Ability3:
				m_ability3OverrideId = abilityOverrideId;
				break;

			default:
				break;
		}

		Set_m_abilityOverrideBitmask(m_abilityOverrideBitmask | static_cast<uint8>(abilityIndex));
	}

	void RemoveAbilityOverride(EAbilityIndex abilityIndex)
	{
		switch (abilityIndex)
		{
		case EAbilityIndex::Ability0:
			m_ability0OverrideId = 0u;
			break;

		case EAbilityIndex::Ability1:
			m_ability1OverrideId = 0u;
			break;

		case EAbilityIndex::Ability2:
			m_ability2OverrideId = 0u;
			break;

		case EAbilityIndex::Ability3:
			m_ability3OverrideId = 0u;
			break;

		default:
			break;
		}

		Set_m_abilityOverrideBitmask(m_abilityOverrideBitmask & ~(static_cast<uint8>(abilityIndex)));
	}

	bool HasAbility(uint32 abilityRecordId) const
	{
		return	GetActiveAbilityId(EAbilityIndex::Ability0) == abilityRecordId ||
				GetActiveAbilityId(EAbilityIndex::Ability1) == abilityRecordId ||
				GetActiveAbilityId(EAbilityIndex::Ability2) == abilityRecordId ||
				GetActiveAbilityId(EAbilityIndex::Ability3) == abilityRecordId;
	}

	bool HasAnyAbility() const
	{
		return	GetActiveAbilityId(EAbilityIndex::Ability0) != 0u ||
				GetActiveAbilityId(EAbilityIndex::Ability1) != 0u ||
				GetActiveAbilityId(EAbilityIndex::Ability2) != 0u ||
				GetActiveAbilityId(EAbilityIndex::Ability3) != 0u;
	}

	bool HasSameAbilities(const AbilityComponent* otherAbilityComponent) const
	{
		if (!otherAbilityComponent)
		{
			return false;
		}

		return	GetActiveAbilityId(EAbilityIndex::Ability0) == otherAbilityComponent->GetActiveAbilityId(EAbilityIndex::Ability0) &&
				GetActiveAbilityId(EAbilityIndex::Ability1) == otherAbilityComponent->GetActiveAbilityId(EAbilityIndex::Ability1) &&
				GetActiveAbilityId(EAbilityIndex::Ability2) == otherAbilityComponent->GetActiveAbilityId(EAbilityIndex::Ability2) &&
				GetActiveAbilityId(EAbilityIndex::Ability3) == otherAbilityComponent->GetActiveAbilityId(EAbilityIndex::Ability3);
	}

	template <typename Function>
	void IterateActiveAbilityIds(Function&& function) const
	{
		function(GetActiveAbilityId(EAbilityIndex::Ability0), EAbilityIndex::Ability0);
		function(GetActiveAbilityId(EAbilityIndex::Ability1), EAbilityIndex::Ability1);
		function(GetActiveAbilityId(EAbilityIndex::Ability2), EAbilityIndex::Ability2);
		function(GetActiveAbilityId(EAbilityIndex::Ability3), EAbilityIndex::Ability3);
	}
};