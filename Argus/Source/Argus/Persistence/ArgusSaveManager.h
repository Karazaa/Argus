// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "ArgusSaveManager.generated.h"

class AArgusGameModeBase;
class USaveGame;
class UArgusSaveGame;
class UArgusMetadataSaveGame;

UCLASS()
class UArgusSaveManager : public UObject 
{
	GENERATED_BODY()

public:
	static UArgusSaveManager* Get() { return k_instance; }

	UArgusSaveManager();
	void BeginDestroy() override;
	
	bool IsSaving() const { return m_saveLockReferenceCount > 0; }
	bool IsLoading() const { return m_loadLockReferenceCount > 0; }
	bool HasLoadRequest() const { return !m_loadRequest.Key.IsEmpty(); }

	void Initialize(const AArgusGameModeBase* gameMode);
	void Save(const TFunction<void(const FString&, bool)>& completedDelegate = nullptr);
	void Load(const FString& saveSlotName, const TFunction<void(UArgusSaveGame*)>& completedDelegate);
	void LoadMostRecent(const TFunction<void(UArgusSaveGame*)>& completedDelegate);

#if !UE_BUILD_SHIPPING
	void DrawDebugger();
#endif //!UE_BUILD_SHIPPING

private:
	enum SaveLoadLockType : uint8
	{
		SaveLock,
		LoadLock
	};

	struct SaveLoadLock
	{
		SaveLoadLock() = default;
		SaveLoadLock(SaveLoadLock&& otherSaveLoadLock) = delete;
		SaveLoadLock(UArgusSaveManager* saveManager, SaveLoadLockType lockType);
		SaveLoadLock(const SaveLoadLock& otherSaveLoadLock);
		~SaveLoadLock();

		SaveLoadLock& operator=(const SaveLoadLock& otherLock) = delete;
		SaveLoadLock& operator=(SaveLoadLock&& otherLock) = delete;

	private:
		void IncrementLock();

		TWeakObjectPtr<UArgusSaveManager> m_saveManager = nullptr;
		SaveLoadLockType m_lockType = SaveLoadLockType::SaveLock;
	};

	static UArgusSaveManager* k_instance;
	static const FString k_metadataSaveSlotName;
	static const FString k_saveSlotPrefix;

	void DoesSaveExistInternal(const FString& saveSlotName, const TFunction<void(const FString&, bool)>& completedDelegate);
	void SaveInternal(const FString& saveSlotName, USaveGame* saveGame, const TFunction<void(bool)>& completedDelegate);
	void ExecuteLoadRequest();
	void LoadInternal(const FString& saveSlotName, const TFunction<void(USaveGame*)>& completedDelegate);
	void OnLoadComplete() const;

	void OnCheckIfMetadataExists(bool doesExist);
	void OnMetadataLoaded(USaveGame* saveGame);

	void OnSaveComplete(const FString& saveSlotName, const SaveLoadLock& SaveLoadLock, bool didSucceed);
	void SaveMetadata(const FString& mostRecentSaveSlotName, const SaveLoadLock& SaveLoadLock);

	void OnCheckIfSaveExists(const FString& saveSlotName, bool doesExist, const TFunction<void(UArgusSaveGame*)>& completedDelegate, const SaveLoadLock& loadLock);

	void PopulateMetadata(const FString& mostRecentSaveSlotName);

	FString GetNextSaveSlotName() const;

	TQueue<TFunction<void(const FString&, bool)>> m_saveRequestQueue;
	TPair<FString, TFunction<void(UArgusSaveGame*)>> m_loadRequest;

	UPROPERTY(Transient)
	TObjectPtr<UArgusMetadataSaveGame> m_saveMetadata = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<const AArgusGameModeBase> m_gameMode = nullptr;

	FPlatformUserId m_userId;
	uint8 m_saveLockReferenceCount = 0u;
	uint8 m_loadLockReferenceCount = 0u;

#if !UE_BUILD_SHIPPING
	int16 m_debugSelectedIndex = -1;
#endif //!UE_BUILD_SHIPPING

	friend class AArgusGameModeBase;
};
