// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameModeBase.h"
#include "ArgusEntity.h"
#include "ArgusGameStateBase.h"
#include "ArgusPlayerController.h"
#include "EngineUtils.h"

AArgusGameModeBase::AArgusGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;

	UE_LOG(ArgusUnrealObjectsLog, Display, TEXT("Constructing Argus game mode base."));
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	Super::StartPlay();
	UE_LOG(ArgusUnrealObjectsLog, Display, TEXT("Argus game mode base starting play."));
}

void AArgusGameModeBase::Tick(float deltaTime)
{
	ARGUS_TRACE(AArgusGameModeBase::Tick)

	Super::Tick(deltaTime);

	UWorld* worldPointer = GetWorld();

	// Update camera and process player input
	for (AArgusPlayerController* argusPlayerController : TActorRange<AArgusPlayerController>(worldPointer))
	{
		if (argusPlayerController)
		{
			argusPlayerController->UpdateCamera(deltaTime);
			argusPlayerController->ProcessArgusPlayerInput();
		}
	}

	// Run all ECS systems.
	m_argusSystemsManager.RunSystems(worldPointer, deltaTime);
}