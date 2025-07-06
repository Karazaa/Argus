// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Widgets/ArgusActorCarrierBarWidget.h"
#include "ArgusEntity.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

void UArgusActorCarrierBarWidget::SetInitialDisplay(const ArgusEntity& entity)
{
	Super::SetInitialDisplay(entity);

	PopulateCarrierSlots(entity);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UArgusActorCarrierBarWidget::RefreshDisplay(const ArgusEntity& entity)
{
	Super::RefreshDisplay(entity);

	const bool isVisible = GetVisibility() != ESlateVisibility::Collapsed;

	if (!entity.IsAlive())
	{
		if (isVisible)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const CarrierComponent* carrierComponent = entity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		if (isVisible)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	if (isVisible && carrierComponent->m_passengerEntityIds.Num() == 0)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!isVisible && carrierComponent->m_passengerEntityIds.Num() > 0)
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (m_currentPassengerCount != carrierComponent->m_passengerEntityIds.Num())
	{
		SetImageColors(carrierComponent);
	}
}

void UArgusActorCarrierBarWidget::PopulateCarrierSlots(const ArgusEntity& entity)
{
	const CarrierComponent* carrierComponent = entity.GetComponent<CarrierComponent>();
	if (!carrierComponent || !m_horizontalBox)
	{
		return;
	}

	m_slotImages.SetNumZeroed(carrierComponent->m_carrierCapacity);
	const int32 passengerCount = carrierComponent->m_passengerEntityIds.Num();
	for (uint8 i = 0u; i < carrierComponent->m_carrierCapacity; ++i)
	{
		m_slotImages[i] = WidgetTree->ConstructWidget<UImage>();
		if (!m_slotImages[i])
		{
			continue;
		}

		UHorizontalBoxSlot* boxSlot = m_horizontalBox->AddChildToHorizontalBox(m_slotImages[i]);
		if (!boxSlot)
		{
			continue;
		}

		boxSlot->SetSize(FSlateChildSize());
		boxSlot->SetPadding(FMargin(1.0f, 1.0f, 1.0f, 1.0f));
	}
}

void UArgusActorCarrierBarWidget::SetImageColors(const CarrierComponent* carrierComponent)
{
	if (!carrierComponent)
	{
		return;
	}

	m_currentPassengerCount = carrierComponent->m_passengerEntityIds.Num();
	for (uint8 i = 0u; i < carrierComponent->m_carrierCapacity; ++i)
	{
		if (!m_slotImages[i])
		{
			continue;
		}

		if (i < m_currentPassengerCount)
		{
			m_slotImages[i]->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			m_slotImages[i]->SetColorAndOpacity(FLinearColor::Black);
		}
	}
}