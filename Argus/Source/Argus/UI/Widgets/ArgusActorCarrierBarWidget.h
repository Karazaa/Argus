// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Views/ArgusActorInfoView.h"
#include "ArgusActorCarrierBarWidget.generated.h"

class ArgusEntity;
class UHorizontalBox;
class UImage;
struct CarrierComponent;

UCLASS()
class UArgusActorCarrierBarWidget : public UArgusActorInfoView
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UHorizontalBox> m_horizontalBox = nullptr;

	UPROPERTY(Transient)
	TArray<UImage*> m_slotImages;

	void PopulateCarrierSlots(const ArgusEntity& entity);
	void SetImageColors(const CarrierComponent* carrierComponent);

	int32 m_currentPassengerCount = 0;
};