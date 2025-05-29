// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "SingleSelectedEntityWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class USingleSelectedEntityWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UImage> m_entityImage = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UTextBlock> m_entityName = nullptr;
};