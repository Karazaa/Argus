// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "RecordDefinitions\PlacedArgusActorTeamInfoRecord.h"
#include "PlacedArgusActorTeamInfoRecordDatabase.generated.h"

UCLASS()
class UPlacedArgusActorTeamInfoRecordDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UPlacedArgusActorTeamInfoRecord* GetRecord(uint32 id);
	void ResizePersistentObjectPointerArray();

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UPlacedArgusActorTeamInfoRecord>> m_UPlacedArgusActorTeamInfoRecords;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPlacedArgusActorTeamInfoRecord>> m_UPlacedArgusActorTeamInfoRecordsPersistent;

public:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext saveContext) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
	void AddUPlacedArgusActorTeamInfoRecordToDatabase(UPlacedArgusActorTeamInfoRecord* record);
#endif //WITH_EDITOR
};
