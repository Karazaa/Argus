// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ReticleActor.h"
#include "Components/DecalComponent.h"

AReticleActor::AReticleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetRootComponent(CreateDefaultSubobject<UDecalComponent>(FName("CameraComponent")));
}