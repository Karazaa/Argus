// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusStaticRecordReference.generated.h"

USTRUCT()
struct FArgusStaticRecordReference
{
	GENERATED_BODY();

	virtual ~FArgusStaticRecordReference() {}
	uint32 GetId() const { return m_id; }

#if WITH_EDITOR
	virtual void StoreId() const {};
#endif

protected:
	UPROPERTY(VisibleAnywhere)
	mutable uint32 m_id = 0;
};