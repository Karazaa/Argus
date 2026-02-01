// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSet.h"

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