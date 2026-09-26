// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusUIElement.h"
#include "SingleSelectedEntityView.generated.h"

class UIconWidget;
class UIconQueueWidget;
class UArgusActorHealthBarWidget;
class UArgusActorCastBarWidget;
class UImage;
class UTextBlock;

UCLASS(meta = (DisableNativeTick))
class USingleSelectedEntityView : public UArgusUIElement
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(ArgusEntity templateEntity) override;
	void OnEntityIconClicked();

protected:
	UPROPERTY(EditAnywhere)
	FSlateBrush m_entityImageSlateBrush;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UIconWidget> m_entityIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> m_entityName = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UArgusActorHealthBarWidget> m_entityHealthBar = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UArgusActorCastBarWidget> m_entityCastBar = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UIconQueueWidget> m_iconQueue = nullptr;

	ArgusEntity m_selectedEntity = ArgusEntity::k_emptyEntity;
};