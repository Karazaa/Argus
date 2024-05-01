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
	m_argusInputManager.SetupInputComponent(InputComponent, m_argusInputActionSet);
}
