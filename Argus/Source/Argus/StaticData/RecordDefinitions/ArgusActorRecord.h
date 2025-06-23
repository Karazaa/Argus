// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntityTemplate.h"
#include "ArgusStaticRecord.h"
#include "Engine/Texture.h"
#include "SoftPtrLoadStore.h"
#include "ArgusActorRecord.generated.h"

class AArgusActor;

UCLASS(BlueprintType)
class ARGUS_API UArgusActorRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AArgusActor> m_argusActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UArgusEntityTemplate> m_entityTemplateOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture> m_actorInfoIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText m_actorInfoName;
};