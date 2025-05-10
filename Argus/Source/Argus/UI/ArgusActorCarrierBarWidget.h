// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorInfoWidget.h"
#include "ArgusActorCarrierBarWidget.generated.h"

class ArgusEntity;
class UHorizontalBox;
class UImage;
struct CarrierComponent;

UCLASS()
class UArgusActorCarrierBarWidget : public UArgusActorInfoWidget
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UHorizontalBox> m_horizontalBox = nullptr;

	void PopulateCarrierSlots(const ArgusEntity& entity);
	void SetImageColors(const CarrierComponent* carrierComponent);

	TArray<UImage*> m_slotImages;
	int32 m_currentPassengerCount = 0;
};