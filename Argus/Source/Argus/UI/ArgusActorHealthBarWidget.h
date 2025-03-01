// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorInfoWidget.h"
#include "ArgusActorHealthBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS()
class UArgusActorHealthBarWidget : public UArgusActorInfoWidget
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity& argusEntity) override;
	virtual void RefreshDisplay(ArgusEntity& argusEntity) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_fullHealthBarColor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor m_lowHealthBarColor;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UProgressBar> m_progressBar = nullptr;

	void SetHealthBarPercentForEntity(ArgusEntity& argusEntity);
};