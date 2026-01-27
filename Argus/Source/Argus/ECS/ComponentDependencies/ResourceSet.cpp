// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSet.h"

FResourceSet FResourceSet::operator-() const
{
	FResourceSet outResources;
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
	{
		outResources.m_resourceQuantities[i] = -m_resourceQuantities[i];
	}

	return outResources;
}

FResourceSet FResourceSet::operator-(const FResourceSet& right) const
{
	FResourceSet outResources;
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
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
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
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
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
	{
		m_resourceQuantities[i] += otherResourceSetRepresentingChange.m_resourceQuantities[i];
	}
}

bool FResourceSet::IsEntirelyAtCap(const FResourceSet& capacityResrouceSet) const 
{
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
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
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
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
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
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
	for (int32 i = 0; i < static_cast<int32>(EResourceType::Count); ++i)
	{
		if (m_resourceQuantities[i] != 0)
		{
			return false;
		}
	}
	return true;
}