// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../ECS/ArgusSystemsManager.h"
#include "ArgusGameModeBase.generated.h"

UCLASS()
class AArgusGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AArgusGameModeBase();
	virtual void StartPlay() override;

protected:
	virtual void Tick(float deltaTime) override;

private:
	ArgusSystemsManager m_argusSystemsManager = ArgusSystemsManager();
};
