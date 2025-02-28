// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceWidget.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Components/TextBlock.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "ResourceWidget"

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

	if (!m_quantityTextBlock)
	{
		ARGUS_LOG(ArgusUILog, Error, TEXT("[%s] %s was unset."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_quantityTextBlock));
		return;
	}
	
	m_quantityTextBlock->SetText(FText::Format(LOCTEXT("ResourceQuantity", "{0}"), newQuantity));
}

#undef LOCTEXT_NAMESPACE // ResourceWidget
