// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveManager.h"
#include "ArgusEntity.h"
#include "ArgusGameModeBase.h"
#include "ArgusMetadataSaveGame.h"
#include "ArgusLogging.h"
#include "ArgusSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

UArgusSaveManager* UArgusSaveManager::k_instance = nullptr;
const FString UArgusSaveManager::k_metadataSaveSlotName = TEXT("ArgusSaveMetadata");
const FString UArgusSaveManager::k_saveSlotPrefix = TEXT("ArgusSave");

UArgusSaveManager::UArgusSaveManager()
{
	m_userId = FPlatformMisc::GetPlatformUserForUserIndex(0);
}

void UArgusSaveManager::BeginDestroy()
{
	k_instance = nullptr;
	Super::BeginDestroy();
}

void UArgusSaveManager::Initialize(const AArgusGameModeBase* gameMode)
{
	ARGUS_RETURN_ON_NULL(gameMode, ArgusPersistenceLog);
	k_instance = this;
	m_gameMode = gameMode;

	DoesSaveExistInternal(k_metadataSaveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this)](const FString& slotName, bool doesExist)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL(rawSaveManager, ArgusPersistenceLog);
		rawSaveManager->OnCheckIfMetadataExists(doesExist);
	});
}

void UArgusSaveManager::Save(const TFunction<void(const FString&, bool)>& completedDelegate)
{
	if (IsSaving())
	{
		m_saveRequestQueue.Enqueue(completedDelegate);
		return;
	}
	const SaveLoadLock saveLock = SaveLoadLock(this, SaveLoadLockType::SaveLock);

	UArgusSaveGame* argusSaveGame = NewObject<UArgusSaveGame>(this);
	const FString saveSlotName = GetNextSaveSlotName();
	ARGUS_RETURN_ON_NULL_INVOKE(argusSaveGame, ArgusPersistenceLog, completedDelegate, saveSlotName, false);

	SaveInternal(saveSlotName, argusSaveGame, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), saveSlotName, saveLock, completedDelegate](bool didSucceed)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL_INVOKE(rawSaveManager, ArgusPersistenceLog, completedDelegate, saveSlotName, didSucceed);

		if (completedDelegate)
		{
			completedDelegate(saveSlotName, didSucceed);
		}

		rawSaveManager->OnSaveComplete(saveSlotName, saveLock, didSucceed);
	});
}

void UArgusSaveManager::Load(const FString& saveSlotName, const TFunction<void(UArgusSaveGame*)>& completedDelegate)
{
	if (IsLoading() || HasLoadRequest())
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (saveSlotName.IsEmpty())
	{
		completedDelegate(nullptr);
		return;
	}

	m_loadRequest.Key = saveSlotName;
	m_loadRequest.Value = completedDelegate;
}

void UArgusSaveManager::LoadMostRecent(const TFunction<void(UArgusSaveGame*)>& completedDelegate)
{
	ARGUS_RETURN_ON_NULL(m_saveMetadata, ArgusPersistenceLog);
	Load(m_saveMetadata->m_mostRecentSaveName, completedDelegate);
}

UArgusSaveManager::SaveLoadLock::SaveLoadLock(UArgusSaveManager* saveManager, SaveLoadLockType lockType)
{
	ARGUS_RETURN_ON_NULL(saveManager, ArgusPersistenceLog);

	m_saveManager = saveManager;
	m_lockType = lockType;
	IncrementLock();
}

UArgusSaveManager::SaveLoadLock::SaveLoadLock(const SaveLoadLock& otherSaveLock)
{
	if (!otherSaveLock.m_saveManager.IsValid())
	{
		return;
	}

	m_saveManager = otherSaveLock.m_saveManager;
	m_lockType = otherSaveLock.m_lockType;
	IncrementLock();
}

UArgusSaveManager::SaveLoadLock::~SaveLoadLock()
{
	if (!m_saveManager.IsValid())
	{
		return;
	}

	switch (m_lockType)
	{
		case SaveLoadLockType::SaveLock:
			{
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
			break;
		case SaveLoadLockType::LoadLock:
			m_saveManager->m_loadLockReferenceCount--;
			break;
		default:
			break;
	}
}

void UArgusSaveManager::SaveLoadLock::IncrementLock()
{
	if (!m_saveManager.IsValid())
	{
		return;
	}

	switch (m_lockType)
	{
		case SaveLoadLockType::SaveLock:
			m_saveManager->m_saveLockReferenceCount++;
			break;
		case SaveLoadLockType::LoadLock:
			m_saveManager->m_loadLockReferenceCount++;
			break;
		default:
			break;
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

void UArgusSaveManager::ExecuteLoadRequest()
{
	if (!HasLoadRequest())
	{
		return;
	}

	ArgusEntity::FlushAllEntities();

	const SaveLoadLock loadLock = SaveLoadLock(this, SaveLoadLockType::LoadLock);
	const FString saveSlotName = m_loadRequest.Key;
	const TFunction<void(UArgusSaveGame*)> completedDelegate = m_loadRequest.Value;

	DoesSaveExistInternal(saveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), completedDelegate, loadLock](const FString& slotName, bool doesExist)
	{
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL_INVOKE(rawSaveManager, ArgusPersistenceLog, completedDelegate, nullptr);
		rawSaveManager->OnCheckIfSaveExists(slotName, doesExist, completedDelegate, loadLock);
	});

	m_loadRequest.Key.Empty();
	m_loadRequest.Value = nullptr;
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

void UArgusSaveManager::OnLoadComplete() const
{
	const AArgusGameModeBase* gameMode = m_gameMode.Get();
	ARGUS_RETURN_ON_NULL(gameMode, ArgusPersistenceLog);
	m_loadCompleted.Broadcast();
	gameMode->OnLoadComplete();
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

void UArgusSaveManager::OnSaveComplete(const FString& saveSlotName, const SaveLoadLock& saveLock, bool didSucceed)
{
	if (!didSucceed)
	{
		return;
	}

	SaveMetadata(saveSlotName, saveLock);
}

void UArgusSaveManager::SaveMetadata(const FString& mostRecentSaveSlotName, const SaveLoadLock& saveLock)
{
	PopulateMetadata(mostRecentSaveSlotName);

	SaveInternal(k_metadataSaveSlotName, m_saveMetadata, [saveLock](bool didSucceed)
	{
		if (!didSucceed)
		{
			ARGUS_LOG(ArgusPersistenceLog, Error, TEXT("[%s] Failed to save metadata!"), ARGUS_FUNCNAME);
		}
	});
}

void UArgusSaveManager::OnCheckIfSaveExists(const FString& saveSlotName, bool doesExist, const TFunction<void(UArgusSaveGame*)>& completedDelegate, const SaveLoadLock& loadLock)
{
	ARGUS_RETURN_ON_NULL(completedDelegate, ArgusPersistenceLog);
	if (!doesExist)
	{
		completedDelegate(nullptr);
		return;
	}

	LoadInternal(saveSlotName, [saveManager = TWeakObjectPtr<UArgusSaveManager>(this), completedDelegate, loadLock](USaveGame* saveGame)
	{
		UArgusSaveGame* argusSaveGame = Cast<UArgusSaveGame>(saveGame);
		ARGUS_RETURN_ON_NULL_INVOKE(argusSaveGame, ArgusPersistenceLog, completedDelegate, argusSaveGame);
		
		UArgusSaveManager* rawSaveManager = saveManager.Get();
		ARGUS_RETURN_ON_NULL_INVOKE(rawSaveManager, ArgusPersistenceLog, completedDelegate, nullptr);
		rawSaveManager->OnLoadComplete();
		
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

FString UArgusSaveManager::GetNextSaveSlotName() const
{
	ARGUS_RETURN_ON_NULL_VALUE(m_saveMetadata, ArgusPersistenceLog, FString());

	return FString::Printf(TEXT("%s_%d"), *k_saveSlotPrefix, m_saveMetadata->m_saveSlotMetadata.Num());
}
