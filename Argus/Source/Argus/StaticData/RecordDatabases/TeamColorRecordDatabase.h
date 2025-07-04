// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusMacros.h"
#include "RecordDefinitions\TeamColorRecord.h"
#include "TeamColorRecordDatabase.generated.h"

UCLASS()
class UTeamColorRecordDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UTeamColorRecord* GetRecord(uint32 id);
	const bool AsyncPreLoadRecord(uint32 id);
	bool ResizePersistentObjectPointerArrayToFitRecord(uint32 id);

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UTeamColorRecord>> m_UTeamColorRecords;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTeamColorRecord>> m_UTeamColorRecordsPersistent;

public:
#if WITH_EDITOR && !IS_PACKAGING_ARGUS
	virtual void PreSave(FObjectPreSaveContext saveContext) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
	void AddUTeamColorRecordToDatabase(UTeamColorRecord* record);
#endif //WITH_EDITOR && !IS_PACKAGING_ARGUS
};
