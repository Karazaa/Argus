// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "ArgusGameInstance.h"
#include "ArgusInputActionSet.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "ArgusUIElement.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "ReticleActor.h"
#include "Slate/SceneViewport.h"

void AArgusPlayerController::ProcessArgusPlayerInput(float deltaTime)
{
	ARGUS_TRACE(AArgusPlayerController::ProcessArgusPlayerInput);

	if (!m_argusInputManager)
	{
		return;
	}

	const AArgusCameraActor::UpdateCameraPanningParameters cameraParams = GetScreenSpaceInputValues();
	const FVector2D mouseScreenSpaceLocation = cameraParams.m_screenSpaceMouseLocation.IsSet() ? cameraParams.m_screenSpaceMouseLocation.GetValue() : FVector2D::ZeroVector;

	m_argusInputManager->ProcessPlayerInput(m_argusCameraActor, cameraParams, deltaTime);
	UpdateUIWidgetDisplay(mouseScreenSpaceLocation);

	if (!m_reticleActor)
	{
		return;
	}

	m_reticleActor->UpdateReticle();
}

AArgusCameraActor::UpdateCameraPanningParameters AArgusPlayerController::GetScreenSpaceInputValues() const
{
	AArgusCameraActor::UpdateCameraPanningParameters output;

	const ULocalPlayer* const localPlayer = GetLocalPlayer();
	if (!localPlayer)
	{
		return output;
	}

	if (!localPlayer->ViewportClient)
	{
		return output;
	}

	FVector2D screenPosition;
	if (localPlayer->ViewportClient->GetMousePosition(screenPosition))
	{
		output.m_screenSpaceMouseLocation = screenPosition;
	}
	if (const FSceneViewport* const gameViewport = localPlayer->ViewportClient->GetGameViewport())
	{
		output.m_screenSpaceXYBounds = gameViewport->GetSizeXY();
	}

	return output;
}

bool AArgusPlayerController::GetMouseProjectionLocation(ECollisionChannel collisionTraceChannel, FHitResult& outHitResult) const
{
	FVector worldSpaceLocation = FVector::ZeroVector;
	FVector worldSpaceDirection = FVector::ZeroVector;

	bool outcome = DeprojectMousePositionToWorld(worldSpaceLocation, worldSpaceDirection);
	FVector traceEndpoint = worldSpaceLocation + (worldSpaceDirection * AArgusCameraActor::k_cameraTraceLength);

	UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL_BOOL(world, ArgusUnrealObjectsLog);

	outcome &= world->LineTraceSingleByChannel(outHitResult, worldSpaceLocation, traceEndpoint, collisionTraceChannel);
	return outcome;
}

AArgusActor* AArgusPlayerController::GetArgusActorForArgusEntityId(uint16 entityId) const 
{
	if (entityId >= ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] passed in %s is greater than or equal to %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(entityId), ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities))
		return nullptr;
	}

	return GetArgusActorForArgusEntity(ArgusEntity::RetrieveEntity(entityId));
}

AArgusActor* AArgusPlayerController::GetArgusActorForArgusEntity(const ArgusEntity& entity) const
{
	ARGUS_RETURN_ON_NULL_POINTER(entity, ArgusUnrealObjectsLog);

	const UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL_POINTER(world, ArgusUnrealObjectsLog);

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL_POINTER(gameInstance, ArgusUnrealObjectsLog);

	return gameInstance->GetArgusActorFromArgusEntity(entity);
}

bool AArgusPlayerController::GetArgusActorsFromArgusEntityIds(const TArray<uint16>& inArgusEntityIds, TArray<AArgusActor*>& outArgusActors) const
{
	outArgusActors.SetNumZeroed(inArgusEntityIds.Num());

	const UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL_BOOL(world, ArgusUnrealObjectsLog);

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL_BOOL(gameInstance, ArgusUnrealObjectsLog);

	for (int i = 0; i < inArgusEntityIds.Num(); ++i)
	{
		outArgusActors[i] = gameInstance->GetArgusActorFromArgusEntity(ArgusEntity::RetrieveEntity(inArgusEntityIds[i]));
	}

	return true;
}

bool AArgusPlayerController::GetArgusActorsFromArgusEntities(const TArray<ArgusEntity>& inArgusEntities, TArray<AArgusActor*>& outArgusActors) const
{
	outArgusActors.SetNumZeroed(inArgusEntities.Num());

	const UWorld* world = GetWorld();
	if (!world)
	{
		return false;
	}

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return false;
	}

	for (int i = 0; i < inArgusEntities.Num(); ++i)
	{
		outArgusActors[i] = gameInstance->GetArgusActorFromArgusEntity(inArgusEntities[i]);
	}

	return true;
}

const UArgusActorRecord* AArgusPlayerController::GetMoveToLocationDecalActorRecord() const
{
	return ArgusStaticData::GetRecord<UArgusActorRecord>(m_moveToLocationDecalActorRecordId);
}

void AArgusPlayerController::FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const
{
	argusActors = argusActors.FilterByPredicate
	(
		[this](AArgusActor* actorToCheck) 
		{
			if (!actorToCheck)
			{
				return false;
			}

			return actorToCheck->GetEntity().IsAlive() && !actorToCheck->GetEntity().IsPassenger() && IsArgusActorOnPlayerTeam(actorToCheck);
		}
	);
}

void AArgusPlayerController::FilterArgusEntityIdsToPlayerTeam(TArray<uint16>& entityIds) const
{
	entityIds = entityIds.FilterByPredicate
	(
		[this](uint16 entityIdToCheck)
		{
			ArgusEntity entityToCheck = ArgusEntity::RetrieveEntity(entityIdToCheck);
			if (!entityToCheck)
			{
				return false;
			}

			return entityToCheck.IsAlive() && !entityToCheck.IsPassenger() && IsArgusEntityOnPlayerTeam(entityToCheck);
		}
	);
}

bool AArgusPlayerController::IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const
{
	ARGUS_RETURN_ON_NULL_BOOL(actor, ArgusUnrealObjectsLog);
	return IsArgusEntityOnPlayerTeam(actor->GetEntity());
}

bool AArgusPlayerController::IsArgusEntityOnPlayerTeam(ArgusEntity entity) const
{
	if (!entity)
	{
		return false;
	}

	const IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return false;
	}

	return identityComponent->m_team == m_playerTeam;
}

void AArgusPlayerController::InitializeUIWidgets()
{
	m_baseCanvasUserWidget = CreateWidget<UArgusUIElement>(this, m_baseCanvasUserWidgetClass, ARGUS_NAMEOF(m_baseCanvasUserWidgetClass));
	if (m_baseCanvasUserWidget)
	{
		m_baseCanvasUserWidget->SetInputManager(m_argusInputManager);
		m_baseCanvasUserWidget->AddToViewport();
	}

	m_selectedArgusEntityUserWidget = CreateWidget<UArgusUIElement>(this, m_selectedArgusEntityUserWidgetClass, ARGUS_NAMEOF(m_selectedArgusEntityUserWidgetClass));
	if (m_selectedArgusEntityUserWidget)
	{
		m_selectedArgusEntityUserWidget->SetInputManager(m_argusInputManager);
		m_selectedArgusEntityUserWidget->AddToViewport();
	}

	m_teamResourcesUserWidget = CreateWidget<UArgusUIElement>(this, m_teamResourcesUserWidgetClass, ARGUS_NAMEOF(m_teamResourcesUserWidgetClass));
	if (m_teamResourcesUserWidget)
	{
		m_teamResourcesUserWidget->SetInputManager(m_argusInputManager);
		m_teamResourcesUserWidget->AddToViewport();
	}
}

void AArgusPlayerController::BeginPlay()
{
	m_argusCameraActor = Cast<AArgusCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArgusCameraActor::StaticClass()));
	if (!m_argusCameraActor)
	{
		m_argusCameraActor = GetWorld()->SpawnActor<AArgusCameraActor>(m_argusCameraClass.LoadSynchronous(), m_defaultInitialCameraTransform.GetLocation(), m_defaultInitialCameraTransform.Rotator());
		ARGUS_RETURN_ON_NULL(m_argusCameraActor, ArgusUnrealObjectsLog);
	}

	m_reticleActor = GetWorld()->SpawnActor<AReticleActor>(m_reticleClass.LoadSynchronous(), m_defaultInitialReticleTransform.GetLocation(), m_defaultInitialReticleTransform.Rotator());
	ARGUS_RETURN_ON_NULL(m_reticleActor, ArgusUnrealObjectsLog);

	SetViewTarget(m_argusCameraActor);
	
	ArgusStaticData::AsyncPreLoadRecord<UArgusActorRecord>(m_moveToLocationDecalActorRecordId);
}

void AArgusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!m_argusInputManager)
	{
		m_argusInputManager = NewObject<UArgusInputManager>();
		ARGUS_RETURN_ON_NULL(m_argusInputManager, ArgusUnrealObjectsLog);
		m_argusInputManager->AddToRoot();
	}

	if (UEnhancedInputLocalPlayerSubsystem* enhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* inputMappingContext = m_argusInputMappingContext.LoadSynchronous())
		{
			enhancedInputSubsystem->AddMappingContext(inputMappingContext, 0);
		}
	}

	m_argusInputManager->SetupInputComponent(this, m_argusInputActionSet);
}

void AArgusPlayerController::UpdateUIWidgetDisplay(const FVector2D& mouseScreenSpaceLocation)
{
	ARGUS_TRACE(AArgusPlayerController::UpdateUIWidgetDisplay);

	UArgusUIElement::UpdateDisplayParameters uiParams = UArgusUIElement::UpdateDisplayParameters(mouseScreenSpaceLocation, m_playerTeam);

	if (m_baseCanvasUserWidget)
	{
		m_baseCanvasUserWidget->UpdateDisplay(uiParams);
	}
	if (m_selectedArgusEntityUserWidget)
	{
		m_selectedArgusEntityUserWidget->UpdateDisplay(uiParams);
	}
	if (m_teamResourcesUserWidget)
	{
		m_teamResourcesUserWidget->UpdateDisplay(uiParams);
	}
}
