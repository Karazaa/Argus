// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "AbilityQueueWidget.generated.h"

class ArgusEntity;
class UImage;
class UUniformGridPanel;
class UUniformGridSlot;

UCLASS()
class UAbilityQueueWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void RefreshDisplay(const ArgusEntity& selectedEntity);

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int32 m_numberOfAbilitiesPerRow = 8;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_abilityImageSlateBrush;

	UPROPERTY(Transient)
	TArray<UUniformGridSlot*> m_gridSlots;
};