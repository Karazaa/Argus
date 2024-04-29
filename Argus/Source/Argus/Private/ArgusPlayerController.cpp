// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "ArgusInputManager.h"
#include "ArgusUtil.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] There is no %s assigned to the %s property of %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusInputManager), ARGUS_NAMEOF(m_argusInputManager), ARGUS_NAMEOF(AArgusPlayerController));
		return;
	}

	m_argusInputManager->SetupInputComponent(InputComponent);
}
