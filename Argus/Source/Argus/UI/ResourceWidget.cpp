// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceWidget.h"

void UResourceWidget::UpdateDisplay(EResourceType resourceType, int32 quantity)
{
	if (!m_currentResourceType.IsSet() || m_currentResourceType.GetValue() != resourceType)
	{
		ChangeResourceType(resourceType);
	}

	if (!m_currentResourceQuantity.IsSet() || m_currentResourceQuantity.GetValue() != quantity)
	{
		ChangeResourceQuantity(quantity);
	}
}

void UResourceWidget::ChangeResourceType(EResourceType newResourceType)
{
	m_currentResourceType = newResourceType;
}

void UResourceWidget::ChangeResourceQuantity(int32 newQuantity)
{
	m_currentResourceQuantity = newQuantity;
}
