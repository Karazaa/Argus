// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "Blueprint/UserWidget.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "ArgusUserWidget.generated.h"

class ArgusEntity;
class UArgusInputManager;

UCLASS()
class UArgusUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	struct UpdateDisplayParameters
	{
		const FVector2D m_currentMouseLocation = FVector2D::ZeroVector;
		const ETeam m_team = ETeam::None;

		UpdateDisplayParameters(const FVector2D& currentMouseLocation, ETeam team) : m_currentMouseLocation(currentMouseLocation), m_team(team) {}
	};

	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams);
	virtual void OnUpdateSelectedArgusActors(ArgusEntity& templateEntity);

	void SetInputManager(UArgusInputManager* inputManager);

protected:
	TWeakObjectPtr<UArgusInputManager> m_inputManager = nullptr;
};