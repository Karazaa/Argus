// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "WorldCellIndexTranslationRecord.generated.h"

class UWorld;

USTRUCT()
struct FCellIndexKey
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 m_cellXIndex = 0;

	UPROPERTY(VisibleAnywhere)
	int32 m_cellYIndex = 0;

	FCellIndexKey() = default;
	FCellIndexKey(int32 cellXIndex, int32 cellYIndex) : m_cellXIndex(cellXIndex), m_cellYIndex(cellYIndex) {}

	bool operator==(const FCellIndexKey& other) const { return m_cellXIndex == other.m_cellXIndex && m_cellYIndex == other.m_cellYIndex; }
};
FORCEINLINE uint32 GetTypeHash(const FCellIndexKey& currentKey) 
{ 
	return HashCombine(GetTypeHash(currentKey.m_cellXIndex), GetTypeHash(currentKey.m_cellYIndex));
}

UCLASS(BlueprintType)
class ARGUS_API UWorldCellIndexTranslationRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> m_worldReference = nullptr;

	UPROPERTY(VisibleAnywhere)
	TMap<FCellIndexKey, uint32> m_cellIndexRecordMapping;
};