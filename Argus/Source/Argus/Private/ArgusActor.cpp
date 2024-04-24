// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActor.h"

AArgusActor::AArgusActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent")));
}

void AArgusActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (m_entityTemplate)
	{
		m_entity = m_entityTemplate->MakeEntity();
	}
}

void AArgusActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		SetActorTransform(transformComponent->m_transform);
	}
}
