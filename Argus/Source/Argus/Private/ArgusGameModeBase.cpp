// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameModeBase.h"
#include "ArgusEntity.h"
#include "ArgusGameStateBase.h"
#include "ArgusPlayerController.h"
#include "ArgusUtil.h"
#include "EngineUtils.h"

AArgusGameModeBase::AArgusGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;

	UE_LOG(ArgusGameLog, Display, TEXT("Constructing Argus game mode base."));
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	Super::StartPlay();
	UE_LOG(ArgusGameLog, Display, TEXT("Argus game mode base starting play."));
}

void AArgusGameModeBase::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	// Process player input
	for (AArgusPlayerController* argusPlayerController : TActorRange<AArgusPlayerController>((GetWorld())))
	{
		if (argusPlayerController)
		{
			argusPlayerController->ProcessArgusPlayerInput();
		}
	}

	// Run all ECS systems.
	m_argusSystemsManager.RunSystems(deltaTime);
}