// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSet.h"

FResourceSet FResourceSet::operator-() const
{
	FResourceSet outResources;
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		outResources.m_resourceQuantities[i] = -m_resourceQuantities[i];
	}

	return outResources;
}

FResourceSet FResourceSet::operator-(const FResourceSet& right) const
{
	FResourceSet outResources;
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		outResources.m_resourceQuantities[i] = m_resourceQuantities[i] - right.m_resourceQuantities[i];
	}

	return outResources;
}

void FResourceSet::Reset()
{
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
	{
		m_resourceQuantities[i] = 0;
	}
}

bool FResourceSet::CanAffordResourceChange(const FResourceSet& otherResourceSetRepresentingChange) const
{
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
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
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		m_resourceQuantities[i] += otherResourceSetRepresentingChange.m_resourceQuantities[i];
	}
}

FResourceSet FResourceSet::CalculateResourceChangeAffordable(const FResourceSet& otherResourceSetRepresentingChange, const FResourceSet* maximumResources) const
{
	FResourceSet changeApplied;
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
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
	const uint8 numResources = static_cast<uint8>(EResourceType::Count);
	for (uint8 i = 0u; i < numResources; ++i)
	{
		if (m_resourceQuantities[i] != 0)
		{
			return false;
		}
	}
	return true;
}