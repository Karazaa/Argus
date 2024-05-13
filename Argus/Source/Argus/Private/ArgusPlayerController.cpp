// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusUtil.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

const float AArgusPlayerController::k_cameraTraceLength = 5000.0f;

void AArgusPlayerController::ProcessArgusPlayerInput()
{
	if (m_argusInputManager)
	{
		m_argusInputManager->ProcessPlayerInput();
	}
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
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
		return false;
	}

	outcome &= world->LineTraceSingleByChannel(outHitResult, worldSpaceLocation, traceEndpoint, ECC_WorldStatic);
	return outcome;
}

void AArgusPlayerController::BeginPlay()
{
	AActor* argusCameraActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArgusCameraActor::StaticClass());
	SetViewTarget(argusCameraActor);
}

void AArgusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!m_argusInputManager)
	{
		m_argusInputManager = NewObject<UArgusInputManager>();
		if (!m_argusInputManager)
		{
			UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to initialize %s when setting up input."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_argusInputManager));
			return;
		}
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
