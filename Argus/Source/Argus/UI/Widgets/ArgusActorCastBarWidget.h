// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Views/ArgusActorInfoView.h"
#include "ArgusActorCastBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS(meta = (DisableNativeTick))
class UArgusActorCastBarWidget : public UArgusActorInfoView
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity argusEntity) override;
	virtual void RefreshDisplay(ArgusEntity argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_abilityCastColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_constructionProgressColor;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> m_castBar = nullptr;
};