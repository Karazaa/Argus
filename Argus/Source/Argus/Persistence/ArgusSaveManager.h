// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusSaveManager.generated.h"

class USaveGame;

UCLASS()
class UArgusSaveManager : public UObject 
{
	GENERATED_BODY()

public:
	UArgusSaveManager();

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

	void DoesSaveExistInternal(const FString& saveSlotName, const TFunction<void(const FString&, bool)> completedDelegate);
	void SaveInternal(const FString& saveSlotName, USaveGame* saveGame, const TFunction<void(bool)>& completedDelegate);
	void LoadInternal(const FString& saveSlotName, const TFunction<void(USaveGame*)>& completedDelegate);

	uint8 m_saveLockReferenceCount = 0u;
	FPlatformUserId m_userId;
};
