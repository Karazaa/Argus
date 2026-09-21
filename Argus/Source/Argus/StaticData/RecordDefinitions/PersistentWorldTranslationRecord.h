// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "PersistentWorldTranslationRecord.generated.h"

class UWorld;

UCLASS(BlueprintType)
class ARGUS_API UPersistentWorldTranslationRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	TMap<TSoftObjectPtr<UWorld>, uint32> m_persistentWorldToRecordId;
};
