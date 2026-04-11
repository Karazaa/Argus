// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "ArgusSaveManager.generated.h"

class USaveGame;
class UArgusSaveGame;
class UArgusMetadataSaveGame;

UCLASS()
class UArgusSaveManager : public UObject 
{
	GENERATED_BODY()

public:
	UArgusSaveManager();

	void Initialize();
	void Save(const TFunction<void(const FString&, bool)>& completedDelegate = nullptr);
	void Load(const FString& saveSlotName, const TFunction<void(UArgusSaveGame*)>& completedDelegate);
	void LoadMostRecent(const TFunction<void(UArgusSaveGame*)>& completedDelegate);

#if !UE_BUILD_SHIPPING
	void DrawDebugger();
#endif //!UE_BUILD_SHIPPING

private:
	struct SaveLock
	{
		SaveLock() = default;
		SaveLock(SaveLock&& otherSaveLock) = delete;
		SaveLock(UArgusSaveManager* saveManager);
		SaveLock(const SaveLock& otherSaveLock);
		~SaveLock();

		SaveLock& operator=(const SaveLock& otherLock) = delete;
		SaveLock& operator=(SaveLock&& otherLock) = delete;

	private:
		TWeakObjectPtr<UArgusSaveManager> m_saveManager = nullptr;
	};

	static const FString k_metadataSaveSlotName;
	static const FString k_saveSlotPrefix;

	void DoesSaveExistInternal(const FString& saveSlotName, const TFunction<void(const FString&, bool)>& completedDelegate);
	void SaveInternal(const FString& saveSlotName, USaveGame* saveGame, const TFunction<void(bool)>& completedDelegate);
	void LoadInternal(const FString& saveSlotName, const TFunction<void(USaveGame*)>& completedDelegate);

	void OnCheckIfMetadataExists(bool doesExist);
	void OnMetadataLoaded(USaveGame* saveGame);

	void OnSaveComplete(const FString& saveSlotName, const SaveLock& saveLock, bool didSucceed);
	void SaveMetadata(const FString& mostRecentSaveSlotName, const SaveLock& saveLock);

	void OnCheckIfSaveExists(const FString& saveSlotName, bool doesExist, const TFunction<void(UArgusSaveGame*)>& completedDelegate);

	void PopulateMetadata(const FString& mostRecentSaveSlotName);

	FString GetNextSaveSlotName() const;

	TQueue<TFunction<void(const FString&, bool)>> m_saveRequestQueue;

	UPROPERTY(Transient)
	TObjectPtr<UArgusMetadataSaveGame> m_saveMetadata = nullptr;

	FPlatformUserId m_userId;
	uint8 m_saveLockReferenceCount = 0u;

#if !UE_BUILD_SHIPPING
	int16 m_debugSelectedIndex = -1;
#endif //!UE_BUILD_SHIPPING
};
