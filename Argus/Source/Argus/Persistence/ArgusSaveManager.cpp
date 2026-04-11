// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveManager.h"
#include "ArgusMetadataSaveGame.h"
#include "ArgusLogging.h"
#include "ArgusSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

const FString UArgusSaveManager::k_metadataSaveSlotName = TEXT("ArgusSaveMetadata");
const FString UArgusSaveManager::k_saveSlotPrefix = TEXT("ArgusSave");

UArgusSaveManager::UArgusSaveManager()
{
	m_userId = FPlatformMisc::GetPlatformUserForUserIndex(0);
}

void UArgusSaveManager::Initialize()
{
	DoesSaveExistInternal(k_metadataSaveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this)](const FString& slotName, bool doesExist)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);
		rawSaveManager->OnCheckIfMetadataExists(doesExist);
	});
}

void UArgusSaveManager::Save(const TFunction<void(const FString&, bool)>& completedDelegate)
{
	if (m_saveLockReferenceCount > 0)
	{
		m_saveRequestQueue.Enqueue(completedDelegate);
		return;
	}
	const SaveLock saveLock = SaveLock(this);

	UArgusSaveGame* argusSaveGame = NewObject<UArgusSaveGame>(this);
	ARGUS_RETURN_ON_NULL(argusSaveGame, ArgusPersistenceLog);

	PopulateSaveGame(argusSaveGame);
	const FString saveSlotName = GetNextSaveSlotName();

	SaveInternal(saveSlotName, argusSaveGame, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), saveSlotName, saveLock, completedDelegate](bool didSucceed)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);

		if (completedDelegate)
		{
			completedDelegate(saveSlotName, didSucceed);
		}

		rawSaveManager->OnSaveComplete(saveSlotName, saveLock, didSucceed);
	});
}

void UArgusSaveManager::Load(const FString& saveSlotName, const TFunction<void(UArgusSaveGame*)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (saveSlotName.IsEmpty())
	{
		completedDelegate(nullptr);
		return;
	}

	DoesSaveExistInternal(saveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), completedDelegate](const FString& slotName, bool doesExist)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);
		rawSaveManager->OnCheckIfSaveExists(slotName, doesExist, completedDelegate);
	});
}

void UArgusSaveManager::LoadMostRecent(const TFunction<void(UArgusSaveGame*)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(m_saveMetadata, ArgusPersistenceLog);
	Load(m_saveMetadata->m_mostRecentSaveName, completedDelegate);
}

UArgusSaveManager::SaveLock::SaveLock(UArgusSaveManager* saveManager)
{
	ARGUS_RETURN_ON_NULL(saveManager, ArgusPersistenceLog);

	m_saveManager = saveManager;
	saveManager->m_saveLockReferenceCount++;
}

UArgusSaveManager::SaveLock::SaveLock(const SaveLock& otherSaveLock)
{
	if (!otherSaveLock.m_saveManager.IsValid())
	{
		return;
	}

	m_saveManager = otherSaveLock.m_saveManager;
	m_saveManager->m_saveLockReferenceCount++;
}

UArgusSaveManager::SaveLock::~SaveLock()
{
	if (!m_saveManager.IsValid())
	{
		return;
	}

	m_saveManager->m_saveLockReferenceCount--;
	if (m_saveManager->m_saveLockReferenceCount > 0u)
	{
		return;
	}

	TFunction<void(const FString&, bool)> cachedDelegate;
	if (m_saveManager->m_saveRequestQueue.Dequeue(cachedDelegate))
	{
		m_saveManager->Save(cachedDelegate);
	}
}

void UArgusSaveManager::DoesSaveExistInternal(const FString& saveSlotName, const TFunction<void(const FString&, bool)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (!ensure(!saveSlotName.IsEmpty()))
	{
		return;
	}

	ISaveGameSystem* saveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	ARGUS_RETURN_ON_NULL(saveSystem, ArgusPersistenceLog);

	saveSystem->DoesSaveGameExistAsync(*saveSlotName, m_userId, [completedDelegate](const FString& saveSlotName, FPlatformUserId userId, ISaveGameSystem::ESaveExistsResult result)
	{
		completedDelegate(saveSlotName, result == ISaveGameSystem::ESaveExistsResult::OK);
	});
}

void UArgusSaveManager::SaveInternal(const FString& saveSlotName, USaveGame* saveGame, const TFunction<void(bool)>& completedDelegate)
{
	if (!ensure(!saveSlotName.IsEmpty()))
	{
		return;
	}

	ISaveGameSystem* saveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	ARGUS_RETURN_ON_NULL(saveSystem, ArgusPersistenceLog);

	TSharedRef<TArray<uint8>> saveBytes = MakeShared<TArray<uint8>>();
	if (!UGameplayStatics::SaveGameToMemory(saveGame, *saveBytes))
	{
		if (completedDelegate)
		{
			completedDelegate(false);
		}
		return;
	}

	saveSystem->SaveGameAsync(false, *saveSlotName, m_userId, saveBytes, [completedDelegate](const FString& saveSlotName, FPlatformUserId userId, bool didSucceed)
	{
		if (completedDelegate)
		{
			completedDelegate(didSucceed);
		}
	});
}

void UArgusSaveManager::LoadInternal(const FString& saveSlotName, const TFunction<void(USaveGame*)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (!ensure(!saveSlotName.IsEmpty()))
	{
		return;
	}

	ISaveGameSystem* saveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	ARGUS_RETURN_ON_NULL(saveSystem, ArgusPersistenceLog);

	saveSystem->LoadGameAsync(false, *saveSlotName, m_userId, [completedDelegate](const FString& saveSlotName, FPlatformUserId userId, bool didSucceed, const TArray<uint8>& data)
	{
		USaveGame* loadedSaveGame = nullptr;
		if (didSucceed)
		{
			loadedSaveGame = UGameplayStatics::LoadGameFromMemory(data);
		}
		if (completedDelegate)
		{
			completedDelegate(loadedSaveGame);
		}
	});
}

void UArgusSaveManager::OnCheckIfMetadataExists(bool doesExist)
{
	if (!doesExist)
	{
		m_saveMetadata = NewObject<UArgusMetadataSaveGame>(this);
		return;
	}

	LoadInternal(k_metadataSaveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this)](USaveGame* saveGame)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);
		rawSaveManager->OnMetadataLoaded(saveGame);
	});
}

void UArgusSaveManager::OnMetadataLoaded(USaveGame* saveGame)
{
	UArgusMetadataSaveGame* metadataSaveGame = Cast<UArgusMetadataSaveGame>(saveGame);
	ARGUS_RETURN_ON_NULL(metadataSaveGame, ArgusPersistenceLog);

	m_saveMetadata = metadataSaveGame;
}

void UArgusSaveManager::OnSaveComplete(const FString& saveSlotName, const SaveLock& saveLock, bool didSucceed)
{
	if (!didSucceed)
	{
		return;
	}

	SaveMetadata(saveSlotName, saveLock);
}

void UArgusSaveManager::SaveMetadata(const FString& mostRecentSaveSlotName, const SaveLock& saveLock)
{
	PopulateMetadata(mostRecentSaveSlotName);

	SaveInternal(k_metadataSaveSlotName, m_saveMetadata, [saveLock](bool didSucceed)
	{
		// TODO JAMES: Error here if needed.
	});
}

void UArgusSaveManager::OnCheckIfSaveExists(const FString& saveSlotName, bool doesExist, const TFunction<void(UArgusSaveGame*)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (!doesExist)
	{
		// TODO JAMES: Error here.
		completedDelegate(nullptr);
		return;
	}

	LoadInternal(saveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), completedDelegate](USaveGame* saveGame)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);
		UArgusSaveGame* argusSaveGame = Cast<UArgusSaveGame>(saveGame);
		ARGUS_RETURN_ON_NULL(argusSaveGame, ArgusPersistenceLog);
		completedDelegate(argusSaveGame);
	});
}

void UArgusSaveManager::PopulateMetadata(const FString& mostRecentSaveSlotName)
{
	ARGUS_RETURN_ON_NULL(m_saveMetadata, ArgusPersistenceLog);

	m_saveMetadata->m_mostRecentSaveName = mostRecentSaveSlotName;
	FSaveSlotMetadata& slotMetadata = m_saveMetadata->m_saveSlotMetadata.Emplace_GetRef();
	slotMetadata.m_slotName = mostRecentSaveSlotName;
	slotMetadata.m_saveTimestamp = FDateTime::Now();
}

void UArgusSaveManager::PopulateSaveGame(UArgusSaveGame* argusSaveGame) const
{
	ARGUS_RETURN_ON_NULL(argusSaveGame, ArgusPersistenceLog);
}

FString UArgusSaveManager::GetNextSaveSlotName() const
{
	ARGUS_RETURN_ON_NULL_VALUE(m_saveMetadata, ArgusPersistenceLog, TEXT("Invalid"));

	return FString::Printf(TEXT("%s_%d"), *k_saveSlotPrefix, m_saveMetadata->m_saveSlotMetadata.Num());
}
