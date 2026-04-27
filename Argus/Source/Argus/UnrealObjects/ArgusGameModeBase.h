// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusActorPool.h"
#include "ArgusSaveManager.h"
#include "ArgusSystemsManager.h"
#include "ArgusSystemsThread.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "ComponentDependencies/ResourceSet.h"
#include "GameFramework/GameModeBase.h"
#include "RecordDefinitions/TeamColorRecord.h"
#include "ArgusGameModeBase.generated.h"

class AArgusPlayerController;
class UArgusEntityTemplate;
class UUserWidget;

UCLASS()
class AArgusGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	FColor GetTeamColor(ETeam team);

	AArgusGameModeBase();
	~AArgusGameModeBase();
	virtual void StartPlay() override;

	AArgusPlayerController* GetActivePlayerController() const { return m_activePlayerController.Get(); }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "EntityTemplates")
	TSoftObjectPtr<UArgusEntityTemplate> m_singletonEntityTemplate = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "EntityTemplates")
	TSoftObjectPtr<UArgusEntityTemplate> m_teamEntityTemplate = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TMap<ETeam, TObjectPtr<UTeamColorRecord>> m_teamColorMap;

	UPROPERTY(Transient)
	TObjectPtr<AArgusPlayerController> m_activePlayerController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UArgusSaveManager> m_saveManager = nullptr;

	virtual void Tick(float deltaTime) override;

private:
	void ManageActorStateForEntities(const UWorld* worldPointer, float deltaTime);
	void SpawnActorForEntity(ArgusEntity spawnedEntity);
	void DespawnActorForEntity(ArgusEntity despawnedEntity);
	void OnLoadStart();
	void OnLoadComplete();

	ArgusActorPool m_argusActorPool = ArgusActorPool();
	ArgusSystemsThread m_argusSystemsThread = ArgusSystemsThread();

	friend class UArgusSaveManager;
};
