// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "Views/ArgusActorInfoView.h"
#include "EntityIconWithInfoWidget.generated.h"

class UArgusInputManager;
class UImage;
struct FSlateBrush;

UCLASS()
class UEntityIconWithInfoWidget : public UArgusActorInfoView
{
	GENERATED_BODY()

public:
	void Populate(ArgusEntity entity, const FSlateBrush& brush, UArgusInputManager* inputManager);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UImage> m_entityIcon = nullptr;

	TWeakObjectPtr<UArgusInputManager> m_inputManager = nullptr;
	ArgusEntity m_trackedEntity = ArgusEntity::k_emptyEntity;
};