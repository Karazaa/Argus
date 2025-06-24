// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "SoftPtrLoadStore.generated.h"

class UArgusEntityTemplate;

USTRUCT(BlueprintType)
struct FSoftPtrLoadStore_UArgusEntityTemplate
{
	GENERATED_BODY()

public:
	UArgusEntityTemplate* LoadAndStorePtr() const
	{
		if (m_hardPtr)
		{
			return m_hardPtr.Get();
		}

		m_hardPtr = m_softPtr.LoadSynchronous();
		return m_hardPtr.Get();
	}

	void SetHardPtr(UArgusEntityTemplate* pointer)
	{
		m_hardPtr = pointer;
	};

	operator bool() const { return m_hardPtr || !m_softPtr.IsNull(); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UArgusEntityTemplate> m_softPtr = nullptr;

	UPROPERTY(Transient)
	mutable TObjectPtr<UArgusEntityTemplate> m_hardPtr = nullptr;
};