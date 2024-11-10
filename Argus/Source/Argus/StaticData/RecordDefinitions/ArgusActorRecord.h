// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntityTemplate.h"
#include "ArgusStaticRecord.h"
#include "ArgusActorRecord.generated.h"

class AArgusActor;

UCLASS(BlueprintType)
class ARGUS_API UArgusActorRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AArgusActor> m_argusActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UArgusEntityTemplate> m_entityTemplateOverride;
};