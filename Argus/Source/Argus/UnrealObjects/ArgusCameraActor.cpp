// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "Camera/CameraComponent.h"

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));
}

void AArgusCameraActor::UpdateCamera(FVector2D screenSpaceMousePosition, FVector2D screenSpaceXYBounds)
{
	const float paddingAmountX = screenSpaceXYBounds.X * 0.2f;
	const float paddingAmountY = screenSpaceXYBounds.Y * 0.2f;

	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;

	if (screenSpaceMousePosition.X < paddingAmountX)
	{
		moveLeft = true;
	}
	else if (screenSpaceMousePosition.X > screenSpaceXYBounds.X - paddingAmountX)
	{
		moveRight = true;
	}

	if (screenSpaceMousePosition.Y < paddingAmountY)
	{
		moveUp = true;
	}
	else if (screenSpaceMousePosition.Y > screenSpaceXYBounds.Y - paddingAmountY)
	{
		moveDown = true;
	}
}