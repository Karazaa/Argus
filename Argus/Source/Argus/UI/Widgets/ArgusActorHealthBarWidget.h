// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Views/ArgusActorInfoView.h"
#include "ArgusActorHealthBarWidget.generated.h"

class ArgusEntity;
class UProgressBar;

UCLASS()
class UArgusActorHealthBarWidget : public UArgusActorInfoView
{
	GENERATED_BODY()

public:
	virtual void SetInitialDisplay(ArgusEntity argusEntity) override;
	virtual void RefreshDisplay(ArgusEntity argusEntity) override;

protected:
	UPROPERTY(EditAnywhere)
	bool m_showHealthBarAtFullHealth = false;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor m_fullHealthBarColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor m_lowHealthBarColor = FLinearColor::Red;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UProgressBar> m_progressBar = nullptr;

	void SetHealthBarPercentForEntity(ArgusEntity argusEntity);
};