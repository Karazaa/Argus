// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveManager.h"
#include "ArgusMetadataSaveGame.h"
#include "ArgusLogging.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

const FString UArgusSaveManager::k_metadataSaveSlotName = TEXT("ArgusSaveMetadata");

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
