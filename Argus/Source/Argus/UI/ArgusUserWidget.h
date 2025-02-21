// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "Blueprint/UserWidget.h"
#include "ArgusUserWidget.generated.h"

class ArgusEntity;
class UArgusInputManager;

UCLASS()
class UArgusUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void OnUpdateSelectedArgusActors(ArgusEntity& templateEntity);

	void SetInputManager(UArgusInputManager* inputManager);

protected:
	TWeakObjectPtr<UArgusInputManager> m_inputManager = nullptr;
};