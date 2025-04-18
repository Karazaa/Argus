// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RecordDatabases/AbilityRecordDatabase.h"
#include "RecordDatabases/ArgusActorRecordDatabase.h"
#include "RecordDatabases/FactionRecordDatabase.h"
#include "RecordDatabases/PlacedArgusActorTeamInfoRecordDatabase.h"
#include "RecordDatabases/ResourceSetRecordDatabase.h"
#include "RecordDatabases/TeamColorRecordDatabase.h"
#include "ArgusStaticDatabase.generated.h"

UCLASS()
class UArgusStaticDatabase : public UDataAsset
{
	GENERATED_BODY()

#pragma region UAbilityRecord
public:
	const UAbilityRecord* GetUAbilityRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUAbilityRecordToDatabase(UAbilityRecord* record);
	void RegisterNewUAbilityRecordDatabase(UAbilityRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UAbilityRecordDatabase> m_UAbilityRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UAbilityRecordDatabase> m_UAbilityRecordDatabasePersistent;

	void LazyLoadUAbilityRecordDatabase();
#pragma endregion
#pragma region UArgusActorRecord
public:
	const UArgusActorRecord* GetUArgusActorRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUArgusActorRecordToDatabase(UArgusActorRecord* record);
	void RegisterNewUArgusActorRecordDatabase(UArgusActorRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UArgusActorRecordDatabase> m_UArgusActorRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UArgusActorRecordDatabase> m_UArgusActorRecordDatabasePersistent;

	void LazyLoadUArgusActorRecordDatabase();
#pragma endregion
#pragma region UFactionRecord
public:
	const UFactionRecord* GetUFactionRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUFactionRecordToDatabase(UFactionRecord* record);
	void RegisterNewUFactionRecordDatabase(UFactionRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFactionRecordDatabase> m_UFactionRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UFactionRecordDatabase> m_UFactionRecordDatabasePersistent;

	void LazyLoadUFactionRecordDatabase();
#pragma endregion
#pragma region UPlacedArgusActorTeamInfoRecord
public:
	const UPlacedArgusActorTeamInfoRecord* GetUPlacedArgusActorTeamInfoRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUPlacedArgusActorTeamInfoRecordToDatabase(UPlacedArgusActorTeamInfoRecord* record);
	void RegisterNewUPlacedArgusActorTeamInfoRecordDatabase(UPlacedArgusActorTeamInfoRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UPlacedArgusActorTeamInfoRecordDatabase> m_UPlacedArgusActorTeamInfoRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UPlacedArgusActorTeamInfoRecordDatabase> m_UPlacedArgusActorTeamInfoRecordDatabasePersistent;

	void LazyLoadUPlacedArgusActorTeamInfoRecordDatabase();
#pragma endregion
#pragma region UResourceSetRecord
public:
	const UResourceSetRecord* GetUResourceSetRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUResourceSetRecordToDatabase(UResourceSetRecord* record);
	void RegisterNewUResourceSetRecordDatabase(UResourceSetRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UResourceSetRecordDatabase> m_UResourceSetRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UResourceSetRecordDatabase> m_UResourceSetRecordDatabasePersistent;

	void LazyLoadUResourceSetRecordDatabase();
#pragma endregion
#pragma region UTeamColorRecord
public:
	const UTeamColorRecord* GetUTeamColorRecord(uint32 id);
#if WITH_EDITOR
	uint32 AddUTeamColorRecordToDatabase(UTeamColorRecord* record);
	void RegisterNewUTeamColorRecordDatabase(UTeamColorRecordDatabase* database);
#endif //WITH_EDITOR

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTeamColorRecordDatabase> m_UTeamColorRecordDatabase;
	UPROPERTY(Transient)
	TObjectPtr<UTeamColorRecordDatabase> m_UTeamColorRecordDatabasePersistent;

	void LazyLoadUTeamColorRecordDatabase();
#pragma endregion

#if WITH_EDITOR
private:
	void SaveDatabase();
#endif // WITH_EDITOR
};
