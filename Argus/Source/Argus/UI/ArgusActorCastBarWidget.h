// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorInfoWidget.h"
#include "ArgusActorCastBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS()
class UArgusActorCastBarWidget : public UArgusActorInfoWidget
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_abilityCastColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_constructionProgressColor;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UProgressBar> m_progressBar = nullptr;
};