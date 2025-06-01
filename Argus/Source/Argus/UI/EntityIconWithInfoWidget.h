// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActorInfoWidget.h"
#include "EntityIconWithInfoWidget.generated.h"

class ArgusEntity;
class UImage;
struct FSlateBrush;

UCLASS()
class UEntityIconWithInfoWidget : public UArgusActorInfoWidget
{
	GENERATED_BODY()

public:
	void Populate(const ArgusEntity& entity, const FSlateBrush& brush);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UImage> m_entityIcon = nullptr;
};