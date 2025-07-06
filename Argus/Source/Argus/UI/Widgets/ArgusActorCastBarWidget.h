// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Views/ArgusActorInfoView.h"
#include "ArgusActorCastBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS()
class UArgusActorCastBarWidget : public UArgusActorInfoView
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(const ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(const ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_abilityCastColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_constructionProgressColor;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UProgressBar> m_progressBar = nullptr;
};