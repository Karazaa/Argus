// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ResourceSet.generated.h"

USTRUCT()
struct FResourceSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float m_resourceAQuantity = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float m_resourceBQuantity = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float m_resourceCQuantity = 0.0f;
};