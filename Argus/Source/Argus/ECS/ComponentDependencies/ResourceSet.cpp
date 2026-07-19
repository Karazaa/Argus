// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSet.h"
#include "Serialization/Archive.h"

#if !UE_BUILD_SHIPPING
#include "imgui.h"
#endif // !UE_BUILD_SHIPPING

FResourceSet FResourceSet::operator-() const
{
	FResourceSet outResources;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		outResources.m_resourceQuantities[i] = -m_resourceQuantities[i];
	}

	return outResources;
}

FResourceSet FResourceSet::operator-(const FResourceSet& right) const
{
	FResourceSet outResources;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		outResources.m_resourceQuantities[i] = m_resourceQuantities[i] - right.m_resourceQuantities[i];
	}

	return outResources;
}

void FResourceSet::Reset()
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		m_resourceQuantities[i] = 0;
	}
}

bool FResourceSet::HasResourceType(EResourceType type) const
{
	return m_resourceQuantities[static_cast<uint8>(type)] > 0;
}

bool FResourceSet::CanAffordResourceChange(const FResourceSet& otherResourceSetRepresentingChange) const
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		if (otherResourceSetRepresentingChange.m_resourceQuantities[i] < 0 &&
			otherResourceSetRepresentingChange.m_resourceQuantities[i] < -m_resourceQuantities[i])
		{
			return false;
		}
	}
	return true;
}

FResourceSet FResourceSet::GetResourceChangeConstraints(const FResourceSet& otherResourceSetRepresentingChange) const
{
	FResourceSet output;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		const int32 change = otherResourceSetRepresentingChange.m_resourceQuantities[i] + m_resourceQuantities[i];
		if (otherResourceSetRepresentingChange.m_resourceQuantities[i] < 0 && change < 0)
		{
			output.m_resourceQuantities[i] = change;
		}
	}

	return output;
}

void FResourceSet::ApplyResourceChange(const FResourceSet& otherResourceSetRepresentingChange)
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		m_resourceQuantities[i] += otherResourceSetRepresentingChange.m_resourceQuantities[i];
	}
}

bool FResourceSet::IsEntirelyAtCap(const FResourceSet& capacityResrouceSet) const 
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		if (m_resourceQuantities[i] < capacityResrouceSet.m_resourceQuantities[i])
		{
			return false;
		}
	}

	return true;
}

bool FResourceSet::IsChangeConstrained(const FResourceSet& constraintResourceSet) const
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		if (m_resourceQuantities[i] < 0 && constraintResourceSet.m_resourceQuantities[i] < 0)
		{
			return true;
		}
	}

	return false;
}

FResourceSet FResourceSet::MaskResourceSet(const FResourceSet& maskSet) const
{
	FResourceSet output = *this;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		if (maskSet.m_resourceQuantities[i] == 0)
		{
			output.m_resourceQuantities[i] = 0;
		}
	}
	return output;
}

FResourceSet FResourceSet::CalculateResourceChangeAffordable(const FResourceSet& otherResourceSetRepresentingChange, const FResourceSet* maximumResources) const
{
	FResourceSet changeApplied;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		const int32 potentialChange = m_resourceQuantities[i] + otherResourceSetRepresentingChange.m_resourceQuantities[i];
		if (maximumResources && potentialChange >= maximumResources->m_resourceQuantities[i])
		{
			changeApplied.m_resourceQuantities[i] = maximumResources->m_resourceQuantities[i] - m_resourceQuantities[i];
		}
		else if (potentialChange < 0)
		{
			changeApplied.m_resourceQuantities[i] = -m_resourceQuantities[i];
		}
		else
		{
			changeApplied.m_resourceQuantities[i] = otherResourceSetRepresentingChange.m_resourceQuantities[i];
		}
	}

	return changeApplied;
}

bool FResourceSet::DoesCostLessThan(const FResourceSet& other) const
{
	int32 sum = 0;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		sum += m_resourceQuantities[i];
	}

	int32 sumOther = 0;
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		sumOther += other.m_resourceQuantities[i];
	}

	return sum > sumOther;
}

bool FResourceSet::IsEmpty() const
{
	for (uint8 i = 0; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		if (m_resourceQuantities[i] != 0)
		{
			return false;
		}
	}
	return true;
}

void FResourceSet::Serialize(FArchive& archive)
{
	for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		archive << m_resourceQuantities[i];
	}
}

#if !UE_BUILD_SHIPPING
void FResourceSet::DrawImGuiDebug() const
{
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (int32 i = 0; i < numResources; ++i)
	{
		ImGui::SameLine();
		ImGui::Text("%d ", m_resourceQuantities[i]);
	}
}
#endif // !UE_BUILD_SHIPPING