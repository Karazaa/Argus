// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "ArgusGameInstance.h"
#include "ArgusInputActionSet.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

const float AArgusPlayerController::k_cameraTraceLength = 5000.0f;

void AArgusPlayerController::UpdateCamera()
{
	if (!m_argusCameraActor)
	{
		return;
	}

	m_argusCameraActor->UpdateCamera();
}

void AArgusPlayerController::ProcessArgusPlayerInput()
{
	if (!m_argusInputManager)
	{
		return;
	}

	m_argusInputManager->ProcessPlayerInput();
}

bool AArgusPlayerController::GetMouseProjectionLocation(FHitResult& outHitResult) const
{
	FVector worldSpaceLocation = FVector::ZeroVector;
	FVector worldSpaceDirection = FVector::ZeroVector;

	bool outcome = DeprojectMousePositionToWorld(worldSpaceLocation, worldSpaceDirection);
	FVector traceEndpoint = worldSpaceLocation + (worldSpaceDirection * k_cameraTraceLength);

	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
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

void AArgusPlayerController::BeginPlay()
{
	m_argusCameraActor = Cast<AArgusCameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArgusCameraActor::StaticClass()));
	if (!m_argusCameraActor)
	{
		UE_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusCameraActor));
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
			UE_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to initialize %s when setting up input."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_argusInputManager));
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
