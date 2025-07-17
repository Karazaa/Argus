// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "IconWidget.h"
#include "ArgusLogging.h"
#include "Components/Image.h"

void UIconWidget::Populate(const TFunction<void(uint16)>& callback, uint16 identifier)
{
	m_onClickedCallback = callback;
	m_identifier = identifier;
}

void UIconWidget::SetBrush(const FSlateBrush& brush)
{
	ARGUS_RETURN_ON_NULL(m_icon, ArgusUILog);
	m_icon->SetBrush(brush);
}

FReply UIconWidget::NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	if (m_onClickedCallback)
	{
		m_onClickedCallback(m_identifier);
	}

	return Super::NativeOnMouseButtonDown(inGeometry, inMouseEvent);
}