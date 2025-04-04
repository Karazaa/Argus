// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "RecordDefinitions\FactionRecord.h"
#include "FactionRecordDatabase.generated.h"

UCLASS()
class UFactionRecordDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UFactionRecord* GetRecord(uint32 id);
	void ResizePersistentObjectPointerArray();

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UFactionRecord>> m_UFactionRecords;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactionRecord>> m_UFactionRecordsPersistent;

public:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext saveContext) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
	void AddUFactionRecordToDatabase(UFactionRecord* record);
#endif //WITH_EDITOR
};
