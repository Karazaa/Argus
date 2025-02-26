// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ResourceSet.generated.h"

UENUM(BlueprintType)
enum class EResourceType : uint8
{
	ResourceA,
	ResourceB,
	ResourceC,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FResourceSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 m_resourceQuantities[(uint8)EResourceType::Count];
};