// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "LocationMemory.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FLocationMemory 
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double m_lastSeen = -9999.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector m_location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double m_lifetime = 30.0f;

	FLocationMemory(double lastSeen = -9999.0, FVector location = FVector::ZeroVector, double lifetime = 30.0)
		: m_lastSeen(lastSeen)
		, m_location(location)
		, m_lifetime(lifetime)
	{
	}

	bool IsExpired(double currentTime) const
	{
		return (currentTime - m_lastSeen) > m_lifetime;
	}
};
