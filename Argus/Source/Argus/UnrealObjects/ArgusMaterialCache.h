// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "SoftPtrLoadStore.h"
#include "ArgusMaterialCache.generated.h"

USTRUCT(BlueprintType)
struct FArgusMaterialCache
{
	GENERATED_BODY()

public:
	static FArgusMaterialCache* s_materialCacheInstance;
	static FArgusMaterialCache* Get() { return s_materialCacheInstance; }

	FArgusMaterialCache();

	UPROPERTY(EditDefaultsOnly)
	FSoftObjectLoadStore_UMaterialInterface m_moveToLocationDecalMaterial;

	UPROPERTY(EditDefaultsOnly)
	FSoftObjectLoadStore_UMaterialInterface m_attackMoveToLocationDecalMaterial;
};