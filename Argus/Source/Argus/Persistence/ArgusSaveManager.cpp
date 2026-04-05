// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSaveManager.h"
#include "ArgusLogging.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

UArgusSaveManager::UArgusSaveManager()
{
	m_userId = FPlatformMisc::GetPlatformUserForUserIndex(0);
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

void UArgusSaveManager::DoesSaveExistInternal(const FString& saveSlotName, const TFunction<void(const FString&, bool)> completedDelegate)
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
