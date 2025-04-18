// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "ResourceSetRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UResourceSetRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FResourceSet m_resourceSet;
};