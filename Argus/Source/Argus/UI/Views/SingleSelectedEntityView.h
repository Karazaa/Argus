// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIElement.h"
#include "SingleSelectedEntityView.generated.h"

class UAbilityQueueWidget;
class UArgusActorHealthBarWidget;
class UImage;
class UTextBlock;

UCLASS()
class USingleSelectedEntityView : public UArgusUIElement
{
	GENERATED_BODY()

public:
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UPROPERTY(EditAnywhere)
	FSlateBrush m_entityImageSlateBrush;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UImage> m_entityImage = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UTextBlock> m_entityName = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UArgusActorHealthBarWidget> m_entityHealthBar = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UAbilityQueueWidget> m_spawnQueue = nullptr;
};