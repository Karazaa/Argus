// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "SoftPtrLoadStore.generated.h"

USTRUCT(BlueprintType)
struct FSoftPtrLoadStore
{
	GENERATED_BODY()

public:
	template<class T>
	T* LoadAndStorePtr()
	{
		if (m_hardPtr)
		{
			return Cast<T>(m_hardPtr.Get());
		}

		m_hardPtr = m_softPtr.LoadSynchronous();
		return Cast<T>(m_hardPtr.Get());
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UObject> m_softPtr = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UObject> m_hardPtr = nullptr;
};