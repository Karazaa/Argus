// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusPlayerController.h"
#include "ArgusCameraActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void AArgusPlayerController::BeginPlay()
{
	AActor* argusCameraActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArgusCameraActor::StaticClass());
	SetViewTarget(argusCameraActor);
}
