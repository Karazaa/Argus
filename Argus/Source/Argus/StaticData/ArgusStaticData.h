// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusGameInstance.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#endif

class ArgusStaticData
{
public:
	template<typename ArgusStaticRecord>
	static const ArgusStaticRecord* GetRecord(uint32 id)
	{
		return nullptr;
	}

#if WITH_EDITOR
private:
	static UArgusStaticDatabase* GetParentDatabase()
	{
		if (!GEditor)
		{
			return nullptr;
		}

		UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
		return Cast<UArgusStaticDatabase>(editorAssetSubsystem->LoadAsset(FString("/Game/StaticData/ArgusStaticDatabase.ArgusStaticDatabase")));
	}

public:
	static uint32 AddRecordToDatabase(UArgusStaticRecord* record)
	{
		UArgusStaticDatabase* staticDatabase = GetParentDatabase();

		if (!staticDatabase)
		{
			return 0u;
		}
		
		if (UAbilityRecord* UAbilityRecordInstance = dynamic_cast<UAbilityRecord*>(record))
		{
			return staticDatabase->AddUAbilityRecordToDatabase(UAbilityRecordInstance);
		}
		if (UArgusActorRecord* UArgusActorRecordInstance = dynamic_cast<UArgusActorRecord*>(record))
		{
			return staticDatabase->AddUArgusActorRecordToDatabase(UArgusActorRecordInstance);
		}
		if (UFactionRecord* UFactionRecordInstance = dynamic_cast<UFactionRecord*>(record))
		{
			return staticDatabase->AddUFactionRecordToDatabase(UFactionRecordInstance);
		}
		if (UPlacedArgusActorTeamInfoRecord* UPlacedArgusActorTeamInfoRecordInstance = dynamic_cast<UPlacedArgusActorTeamInfoRecord*>(record))
		{
			return staticDatabase->AddUPlacedArgusActorTeamInfoRecordToDatabase(UPlacedArgusActorTeamInfoRecordInstance);
		}
		if (UTeamColorRecord* UTeamColorRecordInstance = dynamic_cast<UTeamColorRecord*>(record))
		{
			return staticDatabase->AddUTeamColorRecordToDatabase(UTeamColorRecordInstance);
		}

		return 0u;
	}
#endif //WITH_EDITOR

#pragma region UAbilityRecord
	template<>
	inline const UAbilityRecord* GetRecord(uint32 id)
	{
		UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUAbilityRecord(id);
	}
#pragma endregion
#pragma region UArgusActorRecord
	template<>
	inline const UArgusActorRecord* GetRecord(uint32 id)
	{
		UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUArgusActorRecord(id);
	}
#pragma endregion
#pragma region UFactionRecord
	template<>
	inline const UFactionRecord* GetRecord(uint32 id)
	{
		UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUFactionRecord(id);
	}
#pragma endregion
#pragma region UPlacedArgusActorTeamInfoRecord
	template<>
	inline const UPlacedArgusActorTeamInfoRecord* GetRecord(uint32 id)
	{
		UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUPlacedArgusActorTeamInfoRecord(id);
	}
#pragma endregion
#pragma region UTeamColorRecord
	template<>
	inline const UTeamColorRecord* GetRecord(uint32 id)
	{
		UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUTeamColorRecord(id);
	}

#if WITH_EDITOR
	static void RegisterNewUTeamColorRecordDatabase(const UTeamColorRecordDatabase* database)
	{
		UArgusStaticDatabase* staticDatabase = GetParentDatabase();

		if (!staticDatabase)
		{
			return;
		}

		staticDatabase->RegisterNewUTeamColorRecordDatabase(database);
	}
#endif //WITH_EDITOR
#pragma endregion
};
