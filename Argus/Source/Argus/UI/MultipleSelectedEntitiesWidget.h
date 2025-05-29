// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "MultipleSelectedEntitiesWidget.generated.h"

class UUniformGridPanel;

UCLASS()
class UMultipleSelectedEntitiesWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UUniformGridPanel> m_uniformGridPanel = nullptr;
};