// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUserWidget.h"
#include "SingleSelectedEntityWidget.generated.h"

class UArgusActorHealthBarWidget;
class UImage;
class USpawnQueueWidget;
class UTextBlock;

UCLASS()
class USingleSelectedEntityWidget : public UArgusUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateDisplay(const UpdateDisplayParameters& updateDisplayParams) override;
	virtual void OnUpdateSelectedArgusActors(const ArgusEntity& templateEntity) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush m_entityImageSlateBrush;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UImage> m_entityImage = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UTextBlock> m_entityName = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<UArgusActorHealthBarWidget> m_entityHealthBar = nullptr;

	UPROPERTY(BlueprintReadWrite, Transient)
	TObjectPtr<USpawnQueueWidget> m_spawnQueue = nullptr;
};