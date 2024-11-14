// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusActorPool.h"
#include "ArgusSystemsManager.h"
#include "GameFramework/GameModeBase.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "RecordDefinitions/TeamColorRecord.h"
#include "ArgusGameModeBase.generated.h"

UCLASS()
class AArgusGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	FColor GetTeamColor(ETeam team);

	AArgusGameModeBase();
	virtual void StartPlay() override;

	static void SpawnActorFromEntity(TWeakObjectPtr<UWorld>& worldPointer, ArgusActorPool& argusActorPool, ArgusEntity spawnedEntity);

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TMap<ETeam, TObjectPtr<UTeamColorRecord>> m_teamColorMap;

	virtual void Tick(float deltaTime) override;

private:
	void ManageActorStateForEntities();

	ArgusSystemsManager m_argusSystemsManager = ArgusSystemsManager();
	ArgusActorPool m_argusActorPool = ArgusActorPool();
};
