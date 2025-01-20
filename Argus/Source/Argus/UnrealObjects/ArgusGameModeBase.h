// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusActorPool.h"
#include "ArgusSystemsManager.h"
#include "GameFramework/GameModeBase.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "RecordDefinitions/TeamColorRecord.h"
#include "ArgusGameModeBase.generated.h"

class AArgusPlayerController;
class UUserWidget;

UCLASS()
class AArgusGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	FColor GetTeamColor(ETeam team);

	AArgusGameModeBase();
	virtual void StartPlay() override;

	AArgusPlayerController* GetActivePlayerController() const { return m_activePlayerController.Get(); }

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<ETeam, TObjectPtr<UTeamColorRecord>> m_teamColorMap;

	UPROPERTY(Transient)
	TObjectPtr<AArgusPlayerController> m_activePlayerController;

	virtual void Tick(float deltaTime) override;

private:
	void ManageActorStateForEntities();
	void SpawnActorForEntity(ArgusEntity spawnedEntity);
	void DespawnActorForEntity(ArgusEntity despawnedEntity);

	ArgusSystemsManager m_argusSystemsManager = ArgusSystemsManager();
	ArgusActorPool m_argusActorPool = ArgusActorPool();
};
