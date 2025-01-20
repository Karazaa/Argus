// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "ArgusGameInstance.h"
#include "ArgusInputActionSet.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SceneViewport.h"

void AArgusPlayerController::ProcessArgusPlayerInput(float deltaTime)
{
	if (!m_argusInputManager)
	{
		return;
	}

	m_argusInputManager->ProcessPlayerInput(m_argusCameraActor, GetScreenSpaceInputValues(), deltaTime);
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
		output.m_screenSpaceMousePosition = screenPosition;
	}
	if (const FSceneViewport* const gameViewport = localPlayer->ViewportClient->GetGameViewport())
	{
		output.m_screenSpaceXYBounds = gameViewport->GetSizeXY();
	}

	return output;
}

bool AArgusPlayerController::GetMouseProjectionLocation(FHitResult& outHitResult) const
{
	FVector worldSpaceLocation = FVector::ZeroVector;
	FVector worldSpaceDirection = FVector::ZeroVector;

	bool outcome = DeprojectMousePositionToWorld(worldSpaceLocation, worldSpaceDirection);
	FVector traceEndpoint = worldSpaceLocation + (worldSpaceDirection * AArgusCameraActor::k_cameraTraceLength);

	UWorld* world = GetWorld();
	if (!world)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
		return false;
	}

	outcome &= world->LineTraceSingleByChannel(outHitResult, worldSpaceLocation, traceEndpoint, ECC_WorldStatic);
	return outcome;
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

			return IsArgusActorOnPlayerTeam(actorToCheck);
		}
	);
}

bool AArgusPlayerController::IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const
{
	if (!actor)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Passed %s is null."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusActor*));
		return false;
	}

	ArgusEntity entity = actor->GetEntity();
	if (!entity)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Could not retrieve %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(AArgusActor*));
		return false;
	}

	const IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Could not retrieve %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(IdentityComponent), ARGUS_NAMEOF(ArgusEntity));
		return false;
	}

	return identityComponent->m_team == m_playerTeam;
}

void AArgusPlayerController::InitializeUIWidgets()
{
	UUserWidget* widget = CreateWidget<UUserWidget>(this, m_selectedArgusEntityUserWidgetClass, ARGUS_NAMEOF(m_selectedArgusEntityUserWidgetClass));
	if (widget)
	{
		widget->AddToViewport();
	}
}

void AArgusPlayerController::BeginPlay()
{
	m_argusCameraActor = Cast<AArgusCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArgusCameraActor::StaticClass()));
	if (!m_argusCameraActor)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusCameraActor));
		return;
	}

	SetViewTarget(m_argusCameraActor);
}

void AArgusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!m_argusInputManager)
	{
		m_argusInputManager = NewObject<UArgusInputManager>();
		if (!m_argusInputManager)
		{
			ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to initialize %s when setting up input."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_argusInputManager));
			return;
		}
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
